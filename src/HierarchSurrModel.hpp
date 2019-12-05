/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

#include "SurrogateModel.hpp"
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

class HierarchSurrModel: public SurrogateModel
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

  size_t qoi() const;

  DiscrepancyCorrection& discrepancy_correction();
  short correction_type();
  void  correction_type(short corr_type);

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

  /// return the active low fidelity model
  Model& surrogate_model();
  /// set the key identifying the active low fidelity model
  void surrogate_model_key(unsigned short lf_model_index,
			   unsigned short lf_soln_lev_index = USHRT_MAX);
  /// set the index pair identifying the active low fidelity model
  void surrogate_model_key(const UShortArray& lf_key);

  /// return the active high fidelity model
  Model& truth_model();
  /// set the key identifying the active high fidelity model
  void truth_model_key(unsigned short hf_model_index,
		       unsigned short hf_soln_lev_index = USHRT_MAX);
  /// set the index pair identifying the active high fidelity model
  void truth_model_key(const UShortArray& hf_key);

  /// return pair of active low fidelity and high fidelity model keys
  UShortArrayPair fidelity_keys();

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
  /// Executed by the master to terminate the low and high fidelity model
  /// server operations when iteration on the HierarchSurrModel is complete
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

  /// set the evaluation counter reference points for the HierarchSurrModel
  /// (request forwarded to the low and high fidelity models)
  void set_evaluation_reference();
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

  /// utility for propagating new key values
  void key_updates(unsigned short model_index, unsigned short soln_lev_index);

  /// update sameInterfaceInstance based on interface ids for models
  /// identified by current {low,high}FidelityKey
  void check_model_interface_instance();

  /// update the passed model (one of the ordered models) with data that could
  /// change once per set of evaluations (e.g., an outer iterator execution),
  /// including active variable labels, inactive variable values/bounds/labels,
  /// and linear/nonlinear constraint coeffs/bounds
  void init_model(Model& model);
  /// update the passed model (one of the ordered models) with data that could
  /// change per function evaluation (active variable values/bounds)
  void update_model(Model& model);
  /// update currentVariables using non-active data from the passed model
  /// (one of the ordered models)
  void update_from_model(Model& model);

  /// called from derived_synchronize() and derived_synchronize_nowait() to
  /// extract and rekey response maps using blocking or nonblocking
  /// synchronization on the LF and HF models
  void derived_synchronize_sequential(IntResponseMap& hf_resp_map_rekey,
                                      IntResponseMap& lf_resp_map_rekey,
                                      bool block);
  /// called from derived_synchronize() for case of distinct models/interfaces
  /// with competing LF/HF job queues
  void derived_synchronize_competing();
  /// combine the HF and LF response maps into a combined response map
  void derived_synchronize_combine(const IntResponseMap& hf_resp_map,
                                   IntResponseMap& lf_resp_map,
                                   IntResponseMap& combined_resp_map);
  /// combine the available components from HF and LF response maps
  /// into a combined response map
  void derived_synchronize_combine_nowait(const IntResponseMap& hf_resp_map,
                                          IntResponseMap& lf_resp_map,
                                          IntResponseMap& combined_resp_map);

  /// resize currentResponse based on responseMode
  void resize_response(bool use_virtual_counts = true);

  /// helper function used in the AUTO_CORRECTED_SURROGATE responseMode
  /// for computing a correction and applying it to lf_resp_map
  void compute_apply_delta(IntResponseMap& lf_resp_map);

  /// helper function for applying a single deltaCorr correction
  /// corresponding to key
  void single_apply(const Variables& vars, Response& resp,
		    const UShortArrayPair& keys);
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
  DiscrepCorrMap deltaCorr;
  /// order of correction: 0, 1, or 2
  short corrOrder;

  unsigned short correctionMode;

  /// vector to specify a sequence of discrepancy corrections to apply in
  /// AUTO_CORRECTED_SURROGATE mode
  std::vector<UShortArrayPair> corrSequence;

  /// Ordered sequence (low to high) of model fidelities.  Models are of
  /// arbitrary type and supports recursions.
  ModelArray orderedModels;
  /// flag indicating that the {low,high}FidelityKey correspond to the
  /// same model instance, requiring modifications to updating and evaluation
  /// scheduling processes
  bool sameModelInstance;
  /// flag indicating that the models identified by {low,high}FidelityKey
  /// employ the same interface instance, requiring modifications to evaluation
  /// scheduling processes
  bool sameInterfaceInstance;

  /// store {LF,HF} model key that is active in component_parallel_mode()
  UShortArray componentParallelKey;

  /// map of reference truth (high fidelity) responses computed in
  /// build_approximation() and used for calculating corrections
  std::map<UShortArray, Response> truthResponseRef;
  /// map of truth (high-fidelity) responses retrieved in
  /// derived_synchronize_nowait() that could not be returned since
  /// corresponding low-fidelity response portions were still pending
  IntResponseMap cachedTruthRespMap;
};


inline HierarchSurrModel::~HierarchSurrModel()
{ } // Virtual destructor handles referenceCount at Strategy level.


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
  // forward along to actualModel:
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].nested_variable_mappings(c_index1, di_index1, ds_index1,
					      dr_index1, c_target2, di_target2,
					      ds_target2, dr_target2);
}


inline const SizetArray& HierarchSurrModel::nested_acv1_indices() const
{ return orderedModels[truthModelKey.front()].nested_acv1_indices(); }


inline const ShortArray& HierarchSurrModel::nested_acv2_targets() const
{ return orderedModels[truthModelKey.front()].nested_acv2_targets(); }


inline short HierarchSurrModel::query_distribution_parameter_derivatives() const
{
  return orderedModels[truthModelKey.front()].
    query_distribution_parameter_derivatives();
}


inline size_t HierarchSurrModel::qoi() const
{
  switch (responseMode) {
  // Note: resize_response() aggregates {truth,surrogate}_model().num_fns(),
  //       such that code below is a bit more general that currResp num_fns/2
  case AGGREGATED_MODELS:
    return orderedModels[truthModelKey.front()].qoi();  break;
  default:
    return response_size();                             break;
  }
}


inline void HierarchSurrModel::check_model_interface_instance()
{
  if (surrModelKey.empty() || truthModelKey.empty())
    sameModelInstance = sameInterfaceInstance = false; // even if both empty
  else {
    sameModelInstance = (surrModelKey.front() == truthModelKey.front());

    if (sameModelInstance) sameInterfaceInstance = true;
    else
      sameInterfaceInstance
	= (orderedModels[ surrModelKey.front()].interface_id() ==
	   orderedModels[truthModelKey.front()].interface_id());
  }
}


inline Model& HierarchSurrModel::surrogate_model()
{ return orderedModels[surrModelKey.front()]; }


inline DiscrepancyCorrection& HierarchSurrModel::discrepancy_correction()
{ return deltaCorr[fidelity_keys()]; }


inline short HierarchSurrModel::correction_type()
{ return discrepancy_correction().correction_type(); }


inline void HierarchSurrModel::correction_type(short corr_type)
{
  for (DiscrepCorrMap::iterator it=deltaCorr.begin(); it!=deltaCorr.end(); ++it)
    it->second.correction_type(corr_type);
}


inline void HierarchSurrModel::
key_updates(unsigned short model_index, unsigned short soln_lev_index)
{
  if (soln_lev_index != USHRT_MAX) {
    // Activate variable value for solution control within LF model
    orderedModels[model_index].solution_level_index(soln_lev_index);
    // Pull inactive variable change up into top-level currentVariables,
    // so that data flows correctly within Model recursions?  No, current
    // design is that forward pushes are automated, but inverse pulls are 
    // generally special case invocations from Iterator code (e.g., with
    // locally-managed Model recursions).
    //update_from_model(orderedModels[model_index]);
  }

  // assign same{Model,Interface}Instance
  check_model_interface_instance();

  // *** TO DO: move this to another location, once both keys updated ***
  DiscrepancyCorrection& delta_corr = deltaCorr[fidelity_keys()];
  if (!delta_corr.initialized())
    delta_corr.initialize(surrogate_model(), surrogateFnIndices, corrType,
			  corrOrder);

  // TO DO:
  //deltaCorr.surrogate_model(orderedModels[lf_model_index]);
  //deltaCorr.clear();
}


inline void HierarchSurrModel::
surrogate_model_key(unsigned short lf_model_index,
		    unsigned short lf_soln_lev_index)
{
  SurrogateModel::surrogate_model_key(lf_model_index, lf_soln_lev_index);

  key_updates(lf_model_index, lf_soln_lev_index);
}


inline void HierarchSurrModel::surrogate_model_key(const UShortArray& lf_key)
{
  SurrogateModel::surrogate_model_key(lf_key);

  size_t key_len = lf_key.size();
  unsigned short model_form = (key_len >= 1) ? lf_key[0] : USHRT_MAX,
             soln_lev_index = (key_len >= 2) ? lf_key[1] : USHRT_MAX;
  key_updates(model_form, soln_lev_index);
}


inline Model& HierarchSurrModel::truth_model()
{ return orderedModels[truthModelKey.front()]; }


inline void HierarchSurrModel::
truth_model_key(unsigned short hf_model_index, unsigned short hf_soln_lev_index)
{
  SurrogateModel::truth_model_key(hf_model_index, hf_soln_lev_index);

  key_updates(hf_model_index, hf_soln_lev_index);
}


inline void HierarchSurrModel::truth_model_key(const UShortArray& hf_key)
{
  SurrogateModel::truth_model_key(hf_key);

  size_t key_len = hf_key.size();
  unsigned short model_form = (key_len >= 1) ? hf_key[0] : USHRT_MAX,
             soln_lev_index = (key_len >= 2) ? hf_key[1] : USHRT_MAX;
  key_updates(model_form, soln_lev_index);
}


inline const unsigned short HierarchSurrModel::correction_mode() const
{ return correctionMode; }


inline UShortArrayPair HierarchSurrModel::fidelity_keys()
{ return std::make_pair(surrModelKey, truthModelKey); }


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
    if (depth == std::numeric_limits<size_t>::max())
      lf_model.resize_from_subordinate_model(depth);//retain special value (inf)
    else if (depth)
      lf_model.resize_from_subordinate_model(depth - 1);
  }
  if (hf_resize) {
    Model& hf_model = truth_model();
    if (depth == std::numeric_limits<size_t>::max())
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
    if (depth == std::numeric_limits<size_t>::max())
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
    if (depth == std::numeric_limits<size_t>::max())
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
  responseMode = mode;

  // Trap the combination of no user correction specification with either
  // AUTO_CORRECTED_SURROGATE (NO_CORRECTION defeats the point for HSModel) or
  // MODEL_DISCREPANCY (which formulation for computing discrepancy?) modes.
  if ( !corrType && ( mode == AUTO_CORRECTED_SURROGATE ||
		      mode == MODEL_DISCREPANCY ) ) {
    Cerr << "Error: activation of mode ";
    if (mode == AUTO_CORRECTED_SURROGATE) Cout << "AUTO_CORRECTED_SURROGATE";
    else                                  Cout << "MODEL_DISCREPANCY";
    Cout << " requires specification of a correction type." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  // if necessary, resize the response for entering/exiting an aggregated mode.
  // Since parallel job scheduling only involves either the LF or HF model at
  // any given time, this call does not need to be matched on serve_run() procs.
  resize_response();

  // don't pass to low fidelity model (in case it includes surrogates) since
  // point of a surrogate bypass is to get a surrogate-free truth evaluation
  if (mode == BYPASS_SURROGATE) // recurse in this case
    orderedModels[truthModelKey.front()].surrogate_response_mode(mode);
}


inline void HierarchSurrModel::
surrogate_function_indices(const IntSet& surr_fn_indices)
{ surrogateFnIndices = surr_fn_indices; }


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


inline void HierarchSurrModel::stop_servers()
{ component_parallel_mode(0); }


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


inline void HierarchSurrModel::set_evaluation_reference()
{
  //orderedModels[surrModelKey.front()].set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (responseMode == BYPASS_SURROGATE)
  //  orderedModels[truthModelKey.front()].set_evaluation_reference();

  // may want to add this in time
  //surrModelEvalRef = surrModelEvalCntr;
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
