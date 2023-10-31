/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef ENSEMBLE_SURR_MODEL_H
#define ENSEMBLE_SURR_MODEL_H

#include "SurrogateModel.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"

namespace Dakota {

enum { DEFAULT_CORRECTION = 0, SINGLE_CORRECTION, FULL_MODEL_FORM_CORRECTION,
       FULL_SOLUTION_LEVEL_CORRECTION, SEQUENCE_CORRECTION };


/// Derived model class within the surrogate model branch for managing
/// a truth model alongside approximation models of varying fidelity.

/** The EnsembleSurrModel class manages subordinate models of varying
    fidelity.  The class contains a truth model and an array of
    approximation models, where each model form may also contain a set
    of solution levels (space/time discretization, convergence
    tolerances, etc.). */

class EnsembleSurrModel: public SurrogateModel
{
public:

  //
  //- Heading: Constructors and destructor
  //

  EnsembleSurrModel(ProblemDescDB& problem_db); ///< constructor
  ~EnsembleSurrModel();                         ///< destructor

  //
  //- Heading: Member functions
  //


protected:

  //
  //- Heading: Virtual function redefinitions
  //

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();

  void derived_evaluate(const ActiveSet& set);
  void derived_evaluate_nowait(const ActiveSet& set);

  size_t qoi() const;

  void init_model(Model& model);

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

  const IntResponseMap& derived_synchronize();
  const IntResponseMap& derived_synchronize_nowait();

  void stop_servers();

  bool multifidelity() const;
  bool multilevel() const;
  bool multilevel_multifidelity() const;

  bool multifidelity_precedence() const;
  void multifidelity_precedence(bool mf_prec, bool update_default = true);

  /// set responseMode and pass any bypass request on to the high
  /// fidelity model for any lower-level surrogate recursions
  void surrogate_response_mode(short mode);

  /// (re)set the surrogate index set in SurrogateModel::surrogateFnIndices
  void surrogate_function_indices(const SizetSet& surr_fn_indices);

  // return truthModel interface identifier?
  //const String& interface_id() const;

  /// set the evaluation counter reference points for the EnsembleSurrModel
  /// (request forwarded to truth and surrogate models)
  void set_evaluation_reference();

  void create_tabular_datastream();
  void derived_auto_graphics(const Variables& vars, const Response& resp);

  size_t insert_response_start(size_t position);
  void insert_metadata(const RealArray& md, size_t position,
		       Response& agg_response);

  DiscrepancyCorrection& discrepancy_correction();
  void correction_type(short corr_type);
  unsigned short correction_mode() const;
  void correction_mode(unsigned short corr_mode);

  bool force_rebuild();

  /// use the high fidelity model to compute the truth values needed for
  /// correction of the low fidelity model results
  void build_approximation();
  // Uses the c_vars/response anchor point to define highFidResponse
  //bool build_approximation(const RealVector& c_vars,const Response& response);

  /// return approxModels[i]
  Model& surrogate_model(size_t i = _NPOS);
  /// return approxModels[i]
  const Model& surrogate_model(size_t i = _NPOS) const;
  /// return truthModel
  Model& truth_model();
  /// return truthModel
  const Model& truth_model() const;

  /// return the model form corresponding to surrModelKeys[i]
  unsigned short active_surrogate_model_form(size_t i) const;
  /// return the model form corresponding to truthModelKey
  unsigned short active_truth_model_form() const;

  /// return the model corresponding to surrModelKeys[i] (spanning either
  /// model forms or resolutions)
  Model& active_surrogate_model(size_t i = _NPOS);
  /// return the model corresponding to surrModelKeys[i] (spanning either
  /// model forms or resolutions)
  const Model& active_surrogate_model(size_t i = _NPOS) const;
  /// return the model corresponding to truthModelKey
  Model& active_truth_model();
  /// return the model corresponding to truthModelKey
  const Model& active_truth_model() const;

  /// define the active model key and extract {truth,surr}ModelKeys
  void active_model_key(const Pecos::ActiveKey& key);
  /// remove keys for any approximations underlying {truth,approx}Models
  void clear_model_keys();

  /// return {approxModels,truthModel} and, optionally, their
  /// sub-model recursions
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);

  /// resize currentResponse if needed when one of the subordinate
  /// models has been resized
  void resize_from_subordinate_model(size_t depth = SZ_MAX);
  /// update currentVariables using non-active data from the passed model
  /// (one of {approxModels,truthModel})
  void update_from_subordinate_model(size_t depth = SZ_MAX);

  /// set the relative weightings for multiple objective functions or least
  /// squares terms and optionally recurses into LF/HF models
  void primary_response_fn_weights(const RealVector& wts,
                                   bool recurse_flag = true);

  /// update component parallel mode for supporting parallelism in
  /// the low ad high fidelity models
  void component_parallel_mode(short mode);

  IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// set up parallel operations for the array of subordinate models
  /// {approxModels,truthModel}
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                  bool recurse_flag = true);
  /// set up serial operations for the array of subordinate models
  /// {approxModels,truthModel}
  void derived_init_serial();
  /// set active parallel configuration within the current low and
  /// high fidelity models identified by {low,high}FidelityKey
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                 bool recurse_flag = true);
  /// deallocate communicator partitions for the EnsembleSurrModel
  /// (request forwarded to the the array of subordinate models
  /// {approxModels,truthModel})
  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                  bool recurse_flag = true);

  /// Service the low and high fidelity model job requests received from the
  /// master; completes when termination message received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);

  /// update the Model's inactive view based on higher level (nested)
  /// context and optionally recurse into
  void inactive_view(short view, bool recurse_flag = true);

  /// if recurse_flag, return true if evaluation cache usage in
  /// subordinate models {approxModels,truthModel}
  bool evaluation_cache(bool recurse_flag = true) const;
  /// if recurse_flag, return true if restart file usage in
  /// subordinate models {approxModels,truthModel}
  bool restart_file(bool recurse_flag = true) const;

  /// request fine-grained evaluation reporting within the low and high
  /// fidelity models
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the EnsembleSurrModel
  /// (request forwarded to the low and high fidelity models)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
                                bool relative_count = true) const;

  /// set the warm start flag, including for the subordinate models
  /// {approxModels,truthModel}
  void warm_start_flag(const bool flag);

  //
  //- Heading: member functions
  //

  void derived_synchronize_sequential(
    IntResponseMapArray& model_resp_maps_rekey, bool block);
  void derived_synchronize_combine(IntResponseMapArray& model_resp_maps,
    IntResponseMap& combined_resp_map);
  void derived_synchronize_combine_nowait(IntResponseMapArray& model_resp_maps,
    IntResponseMap& combined_resp_map);

  /// return the model from {approxModels,truthModel} corresponding to m_index
  Model& model_from_index(unsigned short m_index);
  /// return the model from {approxModels,truthModel} corresponding to m_index
  const Model& model_from_index(unsigned short m_index) const;
  /// return approxModels[m_index]
  Model& approx_model_from_index(unsigned short m_index);
  /// return approxModels[m_index]
  const Model& approx_model_from_index(unsigned short m_index) const;

  /// return the key from {truthModel,surrModel}Key{,s} corresponding to k_index
  Pecos::ActiveKey& key_from_index(size_t k_index);

  /// identify whether a model form is currently included within active keys
  bool find_model_in_keys(unsigned short m_index);

  /// distributes the incoming orig_asv among actual_asv and approx_asv
  void asv_split(const ShortArray& orig_asv, ShortArray& approx_asv,
		 ShortArray& actual_asv, bool build_flag = false);
  /// distributes the incoming orig_asv among actual_asv and approx_asv
  void asv_split(const ShortArray& orig_asv, Short2DArray& indiv_asv);

  /// initialize truth and surrogate model keys to default values
  void assign_default_keys(short mode);
  /// size id_maps and cached_resp_maps arrays according to responseMode
  void resize_maps();
  /// resize currentResponse based on responseMode
  void resize_response(bool use_virtual_counts = true);

  /// initialize model variables that corresponsd to nested mappings that could
  /// change once per set of evaluations (e.g., an outer iterator execution)
  void init_model_mapped_variables(Model& model);

  /// called from derived_synchronize() for case of distinct models/interfaces
  /// with competing LF/HF job queues
  void derived_synchronize_competing();

  /// helper to select among Variables::all_discrete_{int,string,real}_
  /// variable_labels() for exporting a solution control variable label
  const String& solution_control_label();

  /// helper to select among Model::solution_level_{int,string,real}_value()
  /// for exporting a scalar solution level value
  void add_tabular_solution_level_value(Model& model);

  //
  //- Heading: Data members
  //

  /// the single truth reference model
  Model truthModel;
  /// set of model approximations
  ModelArray approxModels;

  /// key defining active model form / resolution level for the truth model
  Pecos::ActiveKey truthModelKey;
  /// keys defining model forms / resolution levels for the active set of
  /// approximations
  std::vector<Pecos::ActiveKey> surrModelKeys;

  /// flag indicating that the {low,high}FidelityKey correspond to the
  /// same model instance, requiring modifications to updating and evaluation
  /// scheduling processes
  bool sameModelInstance;
  /// flag indicating that the models identified by {low,high}FidelityKey
  /// employ the same interface instance, requiring modifications to evaluation
  /// scheduling processes
  bool sameInterfaceInstance;
  /// index of solution control variable within all variables
  size_t solnCntlAVIndex;
  /// tie breaker for type of model hierarchy when forms and levels are present
  bool mfPrecedence;

  // store aggregate model key that is active in component_parallel_mode()
  //Pecos::ActiveKey componentParallelKey;
  /// size of MPI buffer containing responseMode and an aggregated activeKey
  int modeKeyBufferSize;

  /// map from evaluation ids of truthModel/approxModels to
  /// EnsembleSurrModel ids
  IntIntMapArray modelIdMaps;
  /// maps of responses retrieved in derived_synchronize_nowait() that
  /// could not be returned since corresponding response portions were
  /// still pending, blocking response aggregation
  IntResponseMapArray cachedRespMaps;

  /// "primary" all continuous variable mapping indices flowed down
  /// from higher level iteration
  SizetArray primaryACVarMapIndices;
  /// "primary" all discrete int variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADIVarMapIndices;
  /// "primary" all discrete string variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADSVarMapIndices;
  /// "primary" all discrete real variable mapping indices flowed down from
  /// higher level iteration
  SizetArray primaryADRVarMapIndices;
  // "secondary" all continuous variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryACVarMapTargets;
  // "secondary" all discrete int variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryADIVarMapTargets;
  // "secondary" all discrete string variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryADSVarMapTargets;
  // "secondary" all discrete real variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryADRVarMapTargets;

private:

  //
  //- Heading: Convenience functions
  //

  /// synchronize the HF model's solution level control with truthModelKey
  void assign_truth_key();
  /// synchronize the LF model's solution level control with surrModelKey
  void assign_surrogate_key(size_t i);
  // assign the resolution level for the model form indicated by the key
  //void assign_key(const Pecos::ActiveKey& key);
  /// assign the resolution level for the i-th model key
  void assign_key(size_t i);

  /// define truth and surrogate keys from incoming active key.  In case of
  /// singleton, use responseMode to disambiguate.
  void extract_truth_key(const Pecos::ActiveKey& active_key,
			 Pecos::ActiveKey& truth_key);
  /// define truth and surrogate keys from incoming active key.  In case of
  /// singleton, use responseMode to disambiguate.
  void extract_subordinate_keys(const Pecos::ActiveKey& active_key,
				std::vector<Pecos::ActiveKey>& surr_keys,
				Pecos::ActiveKey& truth_key);
  // define truth and surrogate keys from incoming active key.  In case of
  // singleton, use component parallel mode to disambiguate.
  //void extract_subordinate_keys(const Pecos::ActiveKey& active_key,
  //			          std::vector<Pecos::ActiveKey>& surr_keys,
  //			          Pecos::ActiveKey& truth_key,
  //			          short parallel_mode);

  /// check for matching interface ids among active truth/surrogate models
  /// (varies based on active keys)
  bool matching_active_interface_ids();
  /// check for matching interface ids across full set of models (invariant)
  bool matching_all_interface_ids();
  /// update sameInterfaceInstance based on interface ids for models
  /// identified by current {low,high}FidelityKey
  void check_model_interface_instance();

  /// compute modeKeyBufferSize
  int server_buffer_size(short mode, const Pecos::ActiveKey& key);
  /// initialize deltaCorr[activeKey]
  void initialize_correction();

  /// stop the servers for the model instance identified by the passed id
  void stop_model(short model_id);

  /// replicate a label array when resizing a response
  void inflate(const StringArray& labels, size_t num_replicates,
	       StringArray& new_labels) const;

  /// check whether incoming ASV has any active content
  bool test_asv(const ShortArray& asv);

  // check whether there are any non-empty maps
  bool test_id_maps(const IntIntMapArray& id_maps);
  // count number of non-empty maps
  size_t count_id_maps(const IntIntMapArray& id_maps);

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

  //
  //- Heading: Data
  //

  /// manages construction and application of correction functions that are
  /// applied to a surrogate model in order to reproduce high fidelity data
  std::map<Pecos::ActiveKey, DiscrepancyCorrection> deltaCorr;
  /// one of {DEFAULT,SINGLE,FULL_SOLUTION_LEVEL,FULL_MODEL_FORM}_CORRECTION
  unsigned short correctionMode;
  // sequence of discrepancy corrections to apply in SEQUENCE_CORRECTION mode
  //std::vector<Pecos::ActiveKey> corrSequence;

  /// map of reference truth (high fidelity) responses computed in
  /// build_approximation() and used for calculating corrections
  std::map<Pecos::ActiveKey, Response> truthResponseRef;

  /// stores a reference copy of the inactive continuous variables when the
  /// approximation is built using a Distinct view; used to detect when a
  /// rebuild is required.
  std::map<Pecos::ActiveKey, RealVector> referenceICVars;
  /// stores a reference copy of the inactive discrete int variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  std::map<Pecos::ActiveKey, IntVector> referenceIDIVars;
  /// stores a reference copy of the inactive discrete string variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  std::map<Pecos::ActiveKey, StringMultiArray> referenceIDSVars;
  /// stores a reference copy of the inactive discrete real variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  std::map<Pecos::ActiveKey, RealVector> referenceIDRVars;

  /// stores a reference copy of active continuous lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  std::map<Pecos::ActiveKey, RealVector> referenceCLBnds;
  /// stores a reference copy of active continuous upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  std::map<Pecos::ActiveKey, RealVector> referenceCUBnds;
  /// stores a reference copy of active discrete int lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  std::map<Pecos::ActiveKey, IntVector> referenceDILBnds;
  /// stores a reference copy of active discrete int upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  std::map<Pecos::ActiveKey, IntVector> referenceDIUBnds;
  /// stores a reference copy of active discrete real lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  std::map<Pecos::ActiveKey, RealVector> referenceDRLBnds;
  /// stores a reference copy of active discrete real upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  std::map<Pecos::ActiveKey, RealVector> referenceDRUBnds;
};


inline EnsembleSurrModel::~EnsembleSurrModel()
{ }


inline size_t EnsembleSurrModel::qoi() const
{
  // resize_response() aggregates {truth,approx} model response fns
  switch (responseMode) {
  case AGGREGATED_MODELS:  case AGGREGATED_MODEL_PAIR:
    return truthModel.qoi();  break;
  default:
    return response_size();   break;
  }
}


inline void EnsembleSurrModel::
inflate(const StringArray& labels, size_t num_replicates,
	StringArray& new_labels) const
{
  size_t i, num_labels = labels.size(), num_new = num_labels * num_replicates;
  new_labels.resize(num_new);
  for (size_t i=0; i<num_new; ++i)
    new_labels[i] = labels[i % num_labels];
}


inline void EnsembleSurrModel::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  primaryACVarMapIndices  = c_index1;
  primaryADIVarMapIndices = di_index1;
  primaryADSVarMapIndices = ds_index1;
  primaryADRVarMapIndices = dr_index1;
  //secondaryACVarMapTargets  = c_target2;
  //secondaryADIVarMapTargets = di_target2;
  //secondaryADSVarMapTargets = ds_target2;
  //secondaryADRVarMapTargets = dr_target2;

  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].nested_variable_mappings(c_index1, di_index1, ds_index1,
					     dr_index1, c_target2, di_target2,
					     ds_target2, dr_target2);

  truthModel.nested_variable_mappings(c_index1, di_index1, ds_index1,
				      dr_index1, c_target2, di_target2,
				      ds_target2, dr_target2);
}


inline const SizetArray& EnsembleSurrModel::nested_acv1_indices() const
{ return primaryACVarMapIndices; }


inline const ShortArray& EnsembleSurrModel::nested_acv2_targets() const
{ return truthModel.nested_acv2_targets(); }//secondaryACVarMapTargets


inline short EnsembleSurrModel::
query_distribution_parameter_derivatives() const
{ return truthModel.query_distribution_parameter_derivatives(); }


inline bool EnsembleSurrModel::force_rebuild()
{
  return check_rebuild(referenceICVars[truthModelKey],
    referenceIDIVars[truthModelKey], referenceIDSVars[truthModelKey],
    referenceIDRVars[truthModelKey], referenceCLBnds[truthModelKey],
    referenceCUBnds[truthModelKey],  referenceDILBnds[truthModelKey],
    referenceDIUBnds[truthModelKey], referenceDRLBnds[truthModelKey],
    referenceDRUBnds[truthModelKey]);
}


inline void EnsembleSurrModel::
check_submodel_compatibility(const Model& sub_model)
{
  bool err1 = check_active_variables(sub_model),
       err2 = check_inactive_variables(sub_model),
       err3 = check_response_qoi(sub_model);

  if (err1 || err2 || err3)
    abort_handler(MODEL_ERROR);
}


inline bool EnsembleSurrModel::multifidelity() const
{
  // This function is used when we don't want to alter logic at run-time based
  // on a deactivated key (as for same{Model,Interface}Instance)
  // > we rely on mfPrecedence passed from NonDExpansion::configure_sequence()
  //   based on the ML/MF algorithm selection; otherwise defaults to true

  return ( approxModels.size() &&
	   ( mfPrecedence || truthModel.solution_levels() <= 1 ) );
}


inline bool EnsembleSurrModel::multilevel() const
{
  return ( truthModel.solution_levels() > 1 &&
	   ( !mfPrecedence || approxModels.empty() ) );
}


inline bool EnsembleSurrModel::multilevel_multifidelity() const
{ return (approxModels.size() && truthModel.solution_levels() > 1); }


inline bool EnsembleSurrModel::multifidelity_precedence() const
{ return mfPrecedence; }


inline void EnsembleSurrModel::
multifidelity_precedence(bool mf_prec, bool update_default)
{
  if (mfPrecedence != mf_prec) {
    mfPrecedence = mf_prec;
    if (update_default) assign_default_keys(responseMode);
  }
}


inline void EnsembleSurrModel::set_evaluation_reference()
{
  //surrogate_model().set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (responseMode == BYPASS_SURROGATE)
  //  truthModel.set_evaluation_reference();

  // may want to add this in time
  //surrModelEvalRef = surrModelEvalCntr;
}


inline bool EnsembleSurrModel::test_id_maps(const IntIntMapArray& id_maps)
{
  size_t i, num_map = id_maps.size();
  for (i=0; i<num_map; ++i)
    if (!id_maps[i].empty())
      return true;
  return false;
}


inline size_t EnsembleSurrModel::count_id_maps(const IntIntMapArray& id_maps)
{
  size_t i, num_map = id_maps.size(), cntr = 0;
  for (i=0; i<num_map; ++i)
    if (!id_maps[i].empty())
      ++cntr;
  return cntr;
}


inline void EnsembleSurrModel::
surrogate_response_mode(short mode)//, bool update_keys)
{
  if (responseMode == mode) return;

  // Note: resize_{response,maps} can require information from {truth,surr}
  // model keys, so we defer resizing until active_model_key(), which
  // generally occurs downstream from (sometimes immediately after) this
  // function.  Iterator::initialize_graphics() --> EnsembleSurrModel::
  // create_tabular_datastream() requires care due to this ordering.
  //
  // tests for outgoing mode:
  //bool resize_for_mode = false;
  //if (responseMode == AGGREGATED_MODELS ||
  //    responseMode == AGGREGATED_MODEL_PAIR)
  //  resize_for_mode = true;

  // now assign new mode
  responseMode = mode;

  // updates for incoming mode:
  switch (mode) {
  case AUTO_CORRECTED_SURROGATE: case MODEL_DISCREPANCY:
    // Trap the omission of a correction specification
    if (!corrType) {
      Cerr << "Error: activation of mode ";
      if (mode == AUTO_CORRECTED_SURROGATE) Cerr << "AUTO_CORRECTED_SURROGATE";
      else                                  Cerr << "MODEL_DISCREPANCY";
      Cerr << " requires specification of a correction type." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    break;
  case BYPASS_SURROGATE:
    // don't propagate to approx models since point of a surrogate bypass
    // is to get a surrogate-free truth evaluation
    truthModel.surrogate_response_mode(mode);  break;
  //case AGGREGATED_MODELS: case AGGREGATED_MODEL_PAIR:
  //  resize_for_mode = true;                    break;
  }

  // if no keys yet, assign default ones for purposes of initialization;
  // these will be replaced at run time
  // > unnecessary if ctor call to assign_default_keys() is active
  //if (update_keys)
  if (truthModelKey.empty() && surrModelKeys.empty())
    assign_default_keys(mode);

  // Defer: surrogate_response_mode() generally precedes activation of keys
  //if (resize_for_mode)
  //  { resize_response(); resize_maps(); }
}


inline int EnsembleSurrModel::
server_buffer_size(short mode, const Pecos::ActiveKey& key)
{
  MPIPackBuffer send_buff;
  send_buff << mode << key; // serve_run() recvs single | aggregate key
  return send_buff.size();
}


inline void EnsembleSurrModel::initialize_correction()
{
  // Correction is required for some responseModes.  Enforcement of a
  // correction type for these modes occurs in surrogate_response_mode().
  if (corrType) { // initialize DiscrepancyCorrection using active key
    DiscrepancyCorrection& delta_corr = deltaCorr[activeKey]; // per data group
    if (!delta_corr.initialized())
      delta_corr.initialize(active_surrogate_model(0), surrogateFnIndices,
			    corrType, corrOrder);
  }
  //truthResponseRef[truthModelKey] = currentResponse.copy();
}


inline void EnsembleSurrModel::
surrogate_function_indices(const SizetSet& surr_fn_indices)
{ surrogateFnIndices = surr_fn_indices; }


inline void EnsembleSurrModel::stop_servers()
{ component_parallel_mode(0); }


inline bool EnsembleSurrModel::matching_all_interface_ids()
{
  size_t i, num_approx = approxModels.size();
  const String& hf_id  = truthModel.interface_id();
  for (i=0; i<num_approx; ++i)
    if (approxModels[i].interface_id() != hf_id)
      return false;
  return true;
}


inline bool EnsembleSurrModel::matching_active_interface_ids()
{
  size_t i, num_approx = surrModelKeys.size();  unsigned short lf_form;
  const String& hf_id  = truthModel.interface_id();
  for (i=0; i<num_approx; ++i) {
    lf_form = surrModelKeys[i].retrieve_model_form();
    if (model_from_index(lf_form).interface_id() != hf_id)
      return false;
  }
  return true;
}


inline void EnsembleSurrModel::check_model_interface_instance()
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
      matching_active_interface_ids();
  }
}


inline size_t EnsembleSurrModel::insert_response_start(size_t position)
{
  size_t i, start = 0, num_approx = surrModelKeys.size();
  if (position > num_approx) {
    Cerr << "Error: invalid position (" << position << ") in EnsembleSurrModel"
	 << "::insert_response_start()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  for (i=0; i<position; ++i) {
    unsigned short form = surrModelKeys[i].retrieve_model_form();
    Model& model_i = model_from_index(form);
    start += model_i.current_response().active_set_request_vector().size();
  }
  return start;
}


inline void EnsembleSurrModel::
insert_metadata(const RealArray& md, size_t position, Response& agg_response)
{
  size_t i, start = 0, num_approx = surrModelKeys.size();
  if (position > num_approx) {
    Cerr << "Error: invalid position (" << position << ") in EnsembleSurrModel"
	 << "::insert_response_start()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  for (i=0; i<position; ++i) {
    unsigned short form = surrModelKeys[i].retrieve_model_form();
    Model& model_i = model_from_index(form);
    start += model_i.current_response().metadata().size();
  }
  agg_response.metadata(md, start);
}


inline Pecos::ActiveKey& EnsembleSurrModel::key_from_index(size_t k_index)
{
  size_t num_approx = surrModelKeys.size();
  if      (k_index <  num_approx) return surrModelKeys[k_index];
  else if (k_index == num_approx) return truthModelKey;
  else { // includes _NPOS
    Cerr << "Error: key index (" << k_index << ") out of range in "
	 << "EnsembleSurrModel::key_from_index()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline Model& EnsembleSurrModel::model_from_index(unsigned short m_index)
{
  size_t num_approx = approxModels.size();
  if      (m_index <  num_approx) return approxModels[m_index];
  else if (m_index == num_approx) return truthModel;
  else { // includes _NPOS
    Cerr << "Error: model index (" << m_index << ") out of range in "
	 << "EnsembleSurrModel::model_from_index()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline const Model& EnsembleSurrModel::
model_from_index(unsigned short m_index) const
{
  size_t num_approx = approxModels.size();
  if      (m_index <  num_approx) return approxModels[m_index];
  else if (m_index == num_approx) return truthModel;
  else { // includes _NPOS
    Cerr << "Error: model index (" << m_index << ") out of range in "
	 << "EnsembleSurrModel::model_from_index()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline Model& EnsembleSurrModel::approx_model_from_index(unsigned short m_index)
{
  size_t num_approx = approxModels.size();
  if (m_index <  num_approx) return approxModels[m_index];
  else { // includes _NPOS
    Cerr << "Error: model index (" << m_index << ") out of range in "
	 << "EnsembleSurrModel::approx_model_from_index()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline const Model& EnsembleSurrModel::
approx_model_from_index(unsigned short m_index) const
{
  size_t num_approx = approxModels.size();
  if (m_index <  num_approx) return approxModels[m_index];
  else { // includes _NPOS
    Cerr << "Error: model index (" << m_index << ") out of range in "
	 << "EnsembleSurrModel::approx_model_from_index()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline bool EnsembleSurrModel::find_model_in_keys(unsigned short m_index)
{
  if (!truthModelKey.empty() && truthModelKey.retrieve_model_form() == m_index)
    return true;
  else {
    size_t k, num_k = surrModelKeys.size();
    for (k=0; k<num_k; ++k)
      if (surrModelKeys[k].retrieve_model_form() == m_index)
	return true;
  }
  return false;
}


/* active cases are Key-based */
inline unsigned short EnsembleSurrModel::
active_surrogate_model_form(size_t i) const
{
  if (i == _NPOS)
    return USHRT_MAX; // defer error/warning/mitigation to calling code
  else if (i >= surrModelKeys.size()) { // hard error
    Cerr << "Error: model form index (" << i << ") out of range ("
	 << surrModelKeys.size() << " active surrogate models) in "
	 << "EnsembleSurrModel::active_surrogate_model_form()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return surrModelKeys[i].retrieve_model_form();
}


inline Model& EnsembleSurrModel::surrogate_model(size_t i)
{
  return (i == _NPOS) ? approx_model_from_index(0)
                      : approx_model_from_index(i);
}


inline const Model& EnsembleSurrModel::surrogate_model(size_t i) const
{
  return (i == _NPOS) ? approx_model_from_index(0)
                      : approx_model_from_index(i);
}


inline unsigned short EnsembleSurrModel::active_truth_model_form() const
{ return truthModelKey.retrieve_model_form(); }


inline Model& EnsembleSurrModel::active_truth_model()
{
  // In ensemble cases, truthModelKey will return truthModel
  // In paired cases, truthModelKey will return the active HF in the pair
  //return truthModel;

  unsigned short hf_form = active_truth_model_form();
  if (hf_form == USHRT_MAX) { // should not happen
    Cerr << "Warning: resorting to default model form in EnsembleSurrModel::"
	 << "truth_model()" << std::endl;
    return truthModel;
  }
  else return model_from_index(hf_form);
}


inline const Model& EnsembleSurrModel::active_truth_model() const
{
  unsigned short hf_form = active_truth_model_form();
  if (hf_form == USHRT_MAX) { // should not happen
    Cerr << "Warning: resorting to default model form in EnsembleSurrModel::"
	 << "truth_model()" << std::endl;
    return truthModel;
  }
  else return model_from_index(hf_form);
}


inline Model& EnsembleSurrModel::truth_model()
{ return truthModel; }


inline const Model& EnsembleSurrModel::truth_model() const
{ return truthModel; }


inline void EnsembleSurrModel::assign_truth_key()
{
  unsigned short hf_form = truthModelKey.retrieve_model_form();
  if (hf_form != USHRT_MAX)
    model_from_index(hf_form).solution_level_cost_index(
      truthModelKey.retrieve_resolution_level());
}


inline void EnsembleSurrModel::assign_surrogate_key(size_t i)
{
  unsigned short lf_form = surrModelKeys[i].retrieve_model_form();
  if (lf_form != USHRT_MAX)
    model_from_index(lf_form).solution_level_cost_index(
      surrModelKeys[i].retrieve_resolution_level());
}


/*
inline void EnsembleSurrModel::assign_key(const Pecos::ActiveKey& key)
{
  unsigned short form = key.retrieve_model_form();
  if (form != USHRT_MAX)
    model_from_index(form).solution_level_cost_index(
      key.retrieve_resolution_level());
}
*/


inline void EnsembleSurrModel::assign_key(size_t i)
{
  if      (i  < surrModelKeys.size()) assign_surrogate_key(i);
  else if (i != _NPOS)                assign_truth_key();
}


inline void EnsembleSurrModel::clear_model_keys()
{
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].clear_model_keys();
  truthModel.clear_model_keys();
}


inline void EnsembleSurrModel::
extract_subordinate_keys(const Pecos::ActiveKey& active_key,
			 std::vector<Pecos::ActiveKey>& surr_keys,
			 Pecos::ActiveKey& truth_key)
{
  if (active_key.aggregated()) // AGGREGATED_MODELS, MODEL_DISCREPANCY
    active_key.extract_keys(surr_keys, truth_key);
  else // singleton key: assign to truth | surr based on responseMode
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      surr_keys.assign(1, active_key); truth_key.clear();  break;
    default: // {BYPASS,NO}_SURROGATE
      truth_key = active_key;          surr_keys.clear();  break;
    }
}


inline void EnsembleSurrModel::
extract_truth_key(const Pecos::ActiveKey& active_key,
		  Pecos::ActiveKey& truth_key)
{
  if (active_key.aggregated()) { // AGGREGATED_MODELS, MODEL_DISCREPANCY
    size_t truth_index = active_key.data_size() - 1; // last key for #keys > 1
    active_key.extract_key(truth_index, truth_key);
  }
  else // singleton key: assign to truth based on responseMode
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      truth_key.clear();      break;
    default: // {BYPASS,NO}_SURROGATE
      truth_key = active_key; break;
    }
}


/*
inline void EnsembleSurrModel::
extract_subordinate_keys(const Pecos::ActiveKey& active_key,
			 std::vector<Pecos::ActiveKey>& surr_keys,
			 Pecos::ActiveKey& truth_key, short parallel_mode)
{
  if (active_key.aggregated())
    active_key.extract_keys(surr_keys, truth_key);
  else// single key: this version assigns to truth | surr based on parallel mode
    switch (parallel_mode) {
    case SURROGATE_MODEL_MODE:
      surr_key.assign(1, active_key);  truth_key.clear();  break;
    case TRUTH_MODEL_MODE:
      truth_key = active_key;          surr_keys.clear();  break;
    }
}


inline bool EnsembleSurrModel::multilevel_from_keys() const
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


inline void EnsembleSurrModel::resize_maps()
{
  size_t num_steps = surrModelKeys.size();
  if (!truthModelKey.empty()) num_steps += 1;
  if (modelIdMaps.size()    != num_steps)    modelIdMaps.resize(num_steps);
  if (cachedRespMaps.size() != num_steps) cachedRespMaps.resize(num_steps);
}


inline bool EnsembleSurrModel::test_asv(const ShortArray& asv)
{
  size_t i, num_fns = asv.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i])
      return true;
  return false;
}


inline void EnsembleSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i) {
    ml.push_back(approxModels[i]);
    if (recurse_flag)
      approxModels[i].derived_subordinate_models(ml, true);
  }
  // models are ordered low to high, so append truth last
  ml.push_back(truthModel);
  if (recurse_flag)
    truthModel.derived_subordinate_models(ml, true);
}


inline void EnsembleSurrModel::resize_from_subordinate_model(size_t depth)
{
  bool all_approx_resize = false, approx0_resize = false, truth_resize = false;
  switch (responseMode) {
  case AGGREGATED_MODELS:
    all_approx_resize = truth_resize = true; break;
  case BYPASS_SURROGATE: case NO_SURROGATE:
    truth_resize = true;                     break;
  case MODEL_DISCREPANCY:
    approx0_resize = truth_resize = true;    break;
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    approx0_resize = true;                   break;
  }

  // bottom-up data flow, so recurse first
  size_t i, num_approx = 0;
  if   (all_approx_resize) num_approx = surrModelKeys.size();
  else if (approx0_resize) num_approx = 1;
  for (i=0; i<num_approx; ++i) {
    Model& model_i = active_surrogate_model(i);
    if (depth == SZ_MAX)
      model_i.resize_from_subordinate_model(depth);// retain special val (inf)
    else if (depth)
      model_i.resize_from_subordinate_model(depth - 1);
  }
  if (truth_resize) {
    Model& truth_model = active_truth_model();
    if (depth == SZ_MAX)
      truth_model.resize_from_subordinate_model(depth);// retain special value
    else if (depth)
      truth_model.resize_from_subordinate_model(depth - 1);
  }
  // now resize this Model's response
  if (all_approx_resize || approx0_resize || truth_resize)
    resize_response(); // resize_maps() ?
}


inline void EnsembleSurrModel::update_from_subordinate_model(size_t depth)
{
  // bottom-up data flow: recurse first, then pull updates from subordinate
  Model& sub_model = ( responseMode ==    UNCORRECTED_SURROGATE ||
		       responseMode == AUTO_CORRECTED_SURROGATE ) ?
    active_surrogate_model(0) : active_truth_model();
  if (depth == SZ_MAX)
    sub_model.update_from_subordinate_model(depth); // retain special value
  else if (depth)
    sub_model.update_from_subordinate_model(depth - 1);
  update_from_model(sub_model);
}


inline void EnsembleSurrModel::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  primaryRespFnWts = wts;
  if (recurse_flag) {
    size_t i, num_approx = approxModels.size();
    for (i=0; i<num_approx; ++i)
      approxModels[i].primary_response_fn_weights(wts, recurse_flag);
    truthModel.primary_response_fn_weights(wts, recurse_flag);
  }
}


inline IntIntPair EnsembleSurrModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // responseMode is a run-time setting, so we are conservative on usage of
  // max_eval_concurrency as in derived_init_communicators()

  probDescDB.set_db_model_nodes(truthModel.model_id());
  IntIntPair min_max_i,
    min_max = truthModel.estimate_partition_bounds(max_eval_concurrency);

  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i) {
    Model& model_i = approxModels[i];
    probDescDB.set_db_model_nodes(model_i.model_id());
    min_max_i = model_i.estimate_partition_bounds(max_eval_concurrency);
    if (min_max_i.first  < min_max.first)  min_max.first  = min_max_i.first;
    if (min_max_i.second > min_max.second) min_max.second = min_max_i.second;
  }

  return min_max;

  // list nodes are reset at the calling level after completion of recursion
}


inline void EnsembleSurrModel::derived_init_serial()
{
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].init_serial();
  truthModel.init_serial();
}


inline void EnsembleSurrModel::stop_model(short model_id)
{
  if (model_id) {
    short  model_index = model_id - 1; // id to index
    Model& model = model_from_index(model_index);
    ParConfigLIter pc_it = model.parallel_configuration_iterator();
    size_t pl_index = model.mi_parallel_level_index();
    if (pc_it->mi_parallel_level_defined(pl_index) &&
	pc_it->mi_parallel_level(pl_index).server_communicator_size() > 1)
      model.stop_servers();
  }
}


inline void EnsembleSurrModel::inactive_view(short view, bool recurse_flag)
{
  currentVariables.inactive_view(view);
  userDefinedConstraints.inactive_view(view);
  if (recurse_flag) {
    size_t i, num_approx = approxModels.size();
    for (i=0; i<num_approx; ++i)
      approxModels[i].inactive_view(view, recurse_flag);
    truthModel.inactive_view(view, recurse_flag);
  }
}


inline bool EnsembleSurrModel::evaluation_cache(bool recurse_flag) const
{
  if (recurse_flag) {
    if (truthModel.evaluation_cache(recurse_flag))
      return true;
    size_t i, num_approx = approxModels.size();
    for (i=0; i<num_approx; ++i)
      if (approxModels[i].evaluation_cache(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline bool EnsembleSurrModel::restart_file(bool recurse_flag) const
{
  if (recurse_flag) {
    if (truthModel.restart_file(recurse_flag))
      return true;
    size_t i, num_approx = approxModels.size();
    for (i=0; i<num_approx; ++i)
      if (approxModels[i].restart_file(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline void EnsembleSurrModel::fine_grained_evaluation_counters()
{
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].fine_grained_evaluation_counters();
  truthModel.fine_grained_evaluation_counters();
}


inline void EnsembleSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
                         bool relative_count) const
{
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].print_evaluation_summary(s, minimal_header, relative_count);
  // emulate low to high ordering
  truthModel.print_evaluation_summary(s, minimal_header, relative_count);
}


inline void EnsembleSurrModel::warm_start_flag(const bool flag)
{
  // Note: supportsEstimDerivs prevents quasi-Newton Hessian accumulations
  warmStartFlag = flag; // for completeness

  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].warm_start_flag(flag);
  truthModel.warm_start_flag(flag);
}


inline DiscrepancyCorrection& EnsembleSurrModel::discrepancy_correction()
{ return deltaCorr[activeKey]; }


inline void EnsembleSurrModel::correction_type(short corr_type)
{
  corrType = corr_type;
  std::map<Pecos::ActiveKey, DiscrepancyCorrection>::iterator it;
  for (it=deltaCorr.begin(); it!=deltaCorr.end(); ++it)
    it->second.correction_type(corr_type);
}


inline unsigned short EnsembleSurrModel::correction_mode() const
{ return correctionMode; }


inline void EnsembleSurrModel::correction_mode(unsigned short corr_mode)
{ correctionMode = corr_mode; }

} // namespace Dakota

#endif
