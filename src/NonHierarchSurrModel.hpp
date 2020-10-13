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

#include "SurrogateModel.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"

namespace Dakota {


/// Derived model class within the surrogate model branch for managing
/// hierarchical surrogates (models of varying fidelity).

/** The NonHierarchSurrModel class manages hierarchical models of varying
    fidelity.  The class contains an ordered array of model forms
    (fidelity ordered from low to high), where each model form may
    also contain a set of solution levels (space/time discretization,
    convergence tolerances, etc.).  At run time, one of these
    combinations is activated as the low fidelity model and used to
    perform approximate function evaluations, while another of these
    combinations is activated as the high fidelity model and used to
    provide truth evaluations for computing corrections to the low
    fidelity results. */

class NonHierarchSurrModel: public SurrogateModel
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

  size_t qoi() const;

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();

  void nested_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2);
  const SizetArray& nested_acv1_indices() const;
  const ShortArray& nested_acv2_targets() const;
  short query_distribution_parameter_derivatives() const;

  void check_submodel_compatibility(const Model& sub_model);

  void derived_evaluate(const ActiveSet& set);
  void derived_evaluate_nowait(const ActiveSet& set);
  const IntResponseMap& derived_synchronize();
  const IntResponseMap& derived_synchronize_nowait();

  // return the active low fidelity model
  //Model& surrogate_model();
  // return the active low fidelity model
  //const Model& surrogate_model() const;

  /// return the high fidelity model
  Model& truth_model();
  /// return the high fidelity model
  const Model& truth_model() const;

  /// define the active model key
  void active_model_key(const UShortArray& key);

  /// return orderedModels and, optionally, their sub-model recursions
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);

  /// resize currentResponse if needed when one of the subordinate
  /// models has been resized
  void resize_from_subordinate_model(size_t depth =
				     std::numeric_limits<size_t>::max());
  /// update currentVariables using non-active data from the passed model
  /// (one of the ordered models)
  void update_from_subordinate_model(size_t depth =
				     std::numeric_limits<size_t>::max());

  /// set the relative weightings for multiple objective functions or least
  /// squares terms and optionally recurses into LF/HF models
  void primary_response_fn_weights(const RealVector& wts,
                                   bool recurse_flag = true);

  /// set responseMode and pass any bypass request on to the high
  /// fidelity model for any lower-level surrogate recursions
  void surrogate_response_mode(short mode);

  /// (re)set the surrogate index set in SurrogateModel::surrogateFnIndices
  void surrogate_function_indices(const IntSet& surr_fn_indices);

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
  /// Executed by the master to terminate the low and high fidelity model
  /// server operations when iteration on the NonHierarchSurrModel is complete
  void stop_servers();

  /// update the Model's inactive view based on higher level (nested)
  /// context and optionally recurse into
  void inactive_view(short view, bool recurse_flag = true);

  // return active orderedModels interface identifier?
  //const String& interface_id() const;
  /// if recurse_flag, return true if orderedModels evaluation cache usage
  bool evaluation_cache(bool recurse_flag = true) const;
  /// if recurse_flag, return true if orderedModels restart file usage
  bool restart_file(bool recurse_flag = true) const;

  /// set the evaluation counter reference points for the NonHierarchSurrModel
  /// (request forwarded to the low and high fidelity models)
  void set_evaluation_reference();
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

  /// utility for propagating new key values
  void key_updates(unsigned short model_index, unsigned short soln_lev_index);

  /// update sameInterfaceInstance based on interface ids for models
  /// identified by current {low,high}FidelityKey
  void check_model_interface_instance();

  /// called from derived_synchronize() and derived_synchronize_nowait() to
  /// extract and rekey response maps using blocking or nonblocking
  /// synchronization on the LF and HF models
  void derived_synchronize_sequential(
    IntResponseMapArray& model_resp_maps_rekey, bool block);
  /// called from derived_synchronize() for case of distinct models/interfaces
  /// with competing LF/HF job queues
  void derived_synchronize_competing();
  /// combine the HF and LF response maps into a combined response map
  void derived_synchronize_combine(const IntResponseMapArray& model_resp_maps,
    IntResponseMap& combined_resp_map, bool block);

  /// resize currentResponse based on responseMode
  void resize_response(bool use_virtual_counts = true);

  /// stop the servers for the model instance identified by the passed id
  void stop_model(short model_id);

  /// check whether incoming ASV has any active content
  bool test_asv(const ShortArray& asv);
  /// check whether there are any non-empty maps
  bool test_id_maps(const IntIntMapArray& id_maps);
  /// count number of non-empty maps
  size_t count_id_maps(const IntIntMapArray& id_maps);

  //
  //- Heading: Data members
  //

  /// the single truth reference model
  Model truthModel;
  /// unordered set of model approximations
  ModelArray unorderedModels;

  //////////////////////////////////////////////////////////////////////////////
  /// W.r.t. active keys, also need to manage active resolutions for each
  /// model instance.  As a first cut, just need to carry them along
  /// (as tunable hyper-parameters rather than dimensions of the hierarchy).
  //////////////////////////////////////////////////////////////////////////////

  /// flag indicating that the {low,high}FidelityKey correspond to the
  /// same model instance, requiring modifications to updating and evaluation
  /// scheduling processes
  bool sameModelInstance;
  /// flag indicating that the models identified by {low,high}FidelityKey
  /// employ the same interface instance, requiring modifications to evaluation
  /// scheduling processes
  bool sameInterfaceInstance;

  /// store aggregate model key that is active in component_parallel_mode()
  UShortArray componentParallelKey;

  // map of reference truth (high fidelity) responses computed in
  // build_approximation() and used for calculating corrections
  //std::map<UShortArray, Response> truthResponseRef;

  IntIntMapArray modelIdMap; // *** TO DO: demote base-class IdMaps or promote MapArray

  /// maps of responses retrieved in derived_synchronize_nowait() that
  /// could not be returned since corresponding response portions were
  /// still pending, blocking response aggregation
  IntResponseMapArray cachedRespMaps; // *** TO DO: demote base-class cache maps or promote MapArray
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
  truthModel.nested_variable_mappings(c_index1, di_index1, ds_index1,
				      dr_index1, c_target2, di_target2,
				      ds_target2, dr_target2);
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i)
    unorderedModels[i].nested_variable_mappings(c_index1, di_index1, ds_index1,
					      dr_index1, c_target2, di_target2,
					      ds_target2, dr_target2);
}


inline const SizetArray& NonHierarchSurrModel::nested_acv1_indices() const
{ return truthModel.nested_acv1_indices(); }


inline const ShortArray& NonHierarchSurrModel::nested_acv2_targets() const
{ return truthModel.nested_acv2_targets(); }


inline short NonHierarchSurrModel::
query_distribution_parameter_derivatives() const
{ return truthModel.query_distribution_parameter_derivatives(); }


inline size_t NonHierarchSurrModel::qoi() const
{
  switch (responseMode) {
  // Note: resize_response() aggregates {truth,surrogate}_model().num_fns(),
  //       such that code below is a bit more general that currResp num_fns/2
  case AGGREGATED_MODELS:  return truthModel.qoi();  break;
  default:                 return response_size();      break;
  }
}


/*
inline void NonHierarchSurrModel::check_model_interface_instance()
{
  unsigned short
    lf_form = ( surrModelKey.empty()) ? USHRT_MAX :  surrModelKey[1],
    hf_form = (truthModelKey.empty()) ? USHRT_MAX : truthModelKey[1];

  if (hf_form == USHRT_MAX || lf_form == USHRT_MAX)
    sameModelInstance = sameInterfaceInstance = false; // including both undef
  else {
    sameModelInstance = (lf_form == hf_form);
    if (sameModelInstance) sameInterfaceInstance = true;
    else
      sameInterfaceInstance
	= (surrogate_model().interface_id() == truth_model().interface_id());
  }
}


inline Model& NonHierarchSurrModel::surrogate_model()
{
  unsigned short lf_form = (surrModelKey.empty()) ? USHRT_MAX : surrModelKey[1];
  if (lf_form == USHRT_MAX) // either empty key or undefined model form
    return orderedModels.front();
  else {
    if (lf_form >= orderedModels.size()) {
      Cerr << "Error: model form (" << lf_form << ") out of range in "
	   << "NonHierarchSurrModel::surrogate_model()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    return orderedModels[lf_form];
  }
}


inline const Model& NonHierarchSurrModel::surrogate_model() const
{
  unsigned short lf_form = (surrModelKey.empty()) ? USHRT_MAX : surrModelKey[1];
  if (lf_form == USHRT_MAX) // either empty key or undefined model form
    return orderedModels.front();
  else {
    if (lf_form >= orderedModels.size()) {
      Cerr << "Error: model form (" << lf_form << ") out of range in "
	   << "NonHierarchSurrModel::surrogate_model()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    return orderedModels[lf_form];
  }
}
*/


inline Model& NonHierarchSurrModel::truth_model()
{ return truthModel; }


inline const Model& NonHierarchSurrModel::truth_model() const
{ return truthModel; }


inline void NonHierarchSurrModel::active_model_key(const UShortArray& key)
{
  // assign activeKey
  SurrogateModel::active_model_key(key);

  /* *** TO DO: activate solution levels within truth/unordered models

  // update {truth,surr}ModelKey
  extract_model_keys(key, truthModelKey, surrModelKey);

  unsigned short hf_form = (truthModelKey.empty()) ? USHRT_MAX:truthModelKey[1],
                 lf_form =  (surrModelKey.empty()) ? USHRT_MAX: surrModelKey[1];
  if (hf_form != lf_form) { // distinct model forms

    // If model forms are distinct (multifidelity), can activate soln level
    // index now; else (multilevel) must defer until run-time.
    if (hf_form != USHRT_MAX)
      orderedModels[hf_form].solution_level_index(truthModelKey[2]);
    if (lf_form != USHRT_MAX)
      orderedModels[lf_form].solution_level_index(surrModelKey[2]);

    // Pull inactive variable change up into top-level currentVariables,
    // so that data flows correctly within Model recursions?  No, current
    // design is that forward pushes are automated, but inverse pulls are 
    // generally special case invocations from Iterator code (e.g., with
    // locally-managed Model recursions).
    //update_from_model(orderedModels[hf_form]);
  }

  // assign same{Model,Interface}Instance
  check_model_interface_instance();
  */
}


inline bool NonHierarchSurrModel::test_asv(const ShortArray& asv)
{
  size_t i, num_fns = asv.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i])
      return true;
  return false;
}


inline bool NonHierarchSurrModel::test_id_maps(const IntIntMapArray& id_maps)
{
  size_t i, num_map = id_maps.size();
  for (i=0; i<num_map; ++i)
    if (!id_maps[i].empty())
      return true;
  return false;
}


inline size_t NonHierarchSurrModel::count_id_maps(const IntIntMapArray& id_maps)
{
  size_t i, num_map = id_maps.size(), cntr = 0;
  for (i=0; i<num_map; ++i)
    if (!id_maps[i].empty())
      ++cntr;
  return cntr;
}


inline void NonHierarchSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i) {
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
    size_t i, num_approx_models = unorderedModels.size();
    for (i=0; i<num_approx_models; ++i) {
      Model& model_i = unorderedModels[i];
      if (depth == std::numeric_limits<size_t>::max())
	model_i.resize_from_subordinate_model(depth);// retain special val (inf)
      else if (depth)
	model_i.resize_from_subordinate_model(depth - 1);
    }
  }
  if (truth_resize) {
    if (depth == std::numeric_limits<size_t>::max())
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
  if (depth == std::numeric_limits<size_t>::max())
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
    truthModel.primary_response_fn_weights(wts, recurse_flag);
    size_t i, num_approx_models = unorderedModels.size();
    for (i=0; i<num_approx_models; ++i)
      unorderedModels[i].primary_response_fn_weights(wts, recurse_flag);
  }
}


inline void NonHierarchSurrModel::surrogate_response_mode(short mode)
{
  responseMode = mode;

  // if necessary, resize the response for entering/exiting an aggregated mode.
  // Since parallel job scheduling only involves either the LF or HF model at
  // any given time, this call does not need to be matched on serve_run() procs.
  resize_response();

  // don't pass to approx models since point of a surrogate bypass is to get
  // a surrogate-free truth evaluation
  if (mode == BYPASS_SURROGATE) // recurse in this case
    truthModel.surrogate_response_mode(mode);
}


inline void NonHierarchSurrModel::
surrogate_function_indices(const IntSet& surr_fn_indices)
{ surrogateFnIndices = surr_fn_indices; }


inline IntIntPair NonHierarchSurrModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // responseMode is a run-time setting, so we are conservative on usage of
  // max_eval_concurrency as in derived_init_communicators()

  size_t i, num_approx_models = unorderedModels.size();
  IntIntPair min_max(INT_MAX, INT_MIN), min_max_i;
  
  probDescDB.set_db_model_nodes(truthModel.model_id());
  min_max_i = truthModel.estimate_partition_bounds(max_eval_concurrency);
  if (min_max_i.first  < min_max.first)  min_max.first  = min_max_i.first;
  if (min_max_i.second > min_max.second) min_max.second = min_max_i.second;

  for (i=0; i<num_approx_models; ++i) {
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
  truthModel.init_serial();
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i)
    unorderedModels[i].init_serial();
}


inline void NonHierarchSurrModel::stop_servers()
{ component_parallel_mode(0); }


inline void NonHierarchSurrModel::stop_model(short model_id)
{
  if (model_id) {
    Model& model = (model_id <= unorderedModels.size()) ?
      unorderedModels[model_id-1] : truthModel;
    ParConfigLIter pc_it = model.parallel_configuration_iterator();
    size_t index = model.mi_parallel_level_index();
    if (pc_it->mi_parallel_level_defined(index) &&
	pc_it->mi_parallel_level(index).server_communicator_size() > 1)
      model.stop_servers();
  }
}


inline void NonHierarchSurrModel::inactive_view(short view, bool recurse_flag)
{
  currentVariables.inactive_view(view);
  userDefinedConstraints.inactive_view(view);
  if (recurse_flag) {
    truthModel.inactive_view(view, recurse_flag);
    size_t i, num_approx_models = unorderedModels.size();
    for (i=0; i<num_approx_models; ++i)
      unorderedModels[i].inactive_view(view, recurse_flag);
  }
}


//inline const String& NonHierarchSurrModel::interface_id() const
//{ return orderedModels[]->interface_id(); }


inline bool NonHierarchSurrModel::evaluation_cache(bool recurse_flag) const
{
  if (recurse_flag) {
    if (truthModel.evaluation_cache(recurse_flag)) return true;
    size_t i, num_approx_models = unorderedModels.size();
    for (i=0; i<num_approx_models; ++i)
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
    if (truthModel.restart_file(recurse_flag)) return true;
    size_t i, num_approx_models = unorderedModels.size();
    for (i=0; i<num_approx_models; ++i)
      if (unorderedModels[i].restart_file(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline void NonHierarchSurrModel::set_evaluation_reference()
{
  //surrogate_model().set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (responseMode == BYPASS_SURROGATE)
  //  truth_model().set_evaluation_reference();

  // may want to add this in time
  //surrModelEvalRef = surrModelEvalCntr;
}


inline void NonHierarchSurrModel::fine_grained_evaluation_counters()
{
  truthModel.fine_grained_evaluation_counters();
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i)
    unorderedModels[i].fine_grained_evaluation_counters();
}


inline void NonHierarchSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
                         bool relative_count) const
{
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i)
    unorderedModels[i].print_evaluation_summary(s, minimal_header,
						relative_count);
  // emulate low to high ordering as in HierarchSurrModel
  truthModel.print_evaluation_summary(s, minimal_header, relative_count);
}


inline void NonHierarchSurrModel::warm_start_flag(const bool flag)
{
  // Note: supportsEstimDerivs prevents quasi-Newton Hessian accumulations
  warmStartFlag = flag; // for completeness

  truthModel.warm_start_flag(flag);
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i)
    unorderedModels[i].warm_start_flag(flag);
}

} // namespace Dakota

#endif
