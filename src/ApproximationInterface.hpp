/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef APPROXIMATION_INTERFACE_H
#define APPROXIMATION_INTERFACE_H

#include "SharedApproxData.hpp"
#include "DakotaApproximation.hpp"
#include "DakotaInterface.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "PRPMultiIndex.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Derived class within the interface class hierarchy for supporting
/// approximations to simulation-based results.

/** ApproximationInterface provides an interface class for building a
    set of global/local/multipoint approximations and performing
    approximate function evaluations using them.  It contains a list
    of Approximation objects, one for each response function. */

class ApproximationInterface: public Interface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// primary constructor
  ApproximationInterface(ProblemDescDB& problem_db, const Variables& am_vars,
			 bool am_cache, const String& am_interface_id,
			 const StringArray& fn_labels);
  /// alternate constructor for instantiations on the fly
  ApproximationInterface(const String& approx_type,
			 const UShortArray& approx_order,
			 const Variables& am_vars, bool am_cache,
			 const String& am_interface_id, size_t num_fns,
			 short data_order, short output_level);
  /// destructor
  ~ApproximationInterface();

protected:

  //
  //- Heading: Methods (protected due to letter-envelope idiom)
  //

  /// return reference to specified approximation
  virtual Approximation& function_surface(size_t);
  virtual const Approximation& function_surface(size_t) const;

  /// return surface field component for response index
  virtual int field_component(size_t fn_index) const;

  virtual size_t num_function_surfaces() const;

  /// the function evaluator: provides an approximate "mapping" from
  /// the variables to the responses using functionSurfaces
  void map(const Variables& vars, const ActiveSet& set, Response& response,
	   bool asynch_flag = false);

  /// returns the minimum number of samples required to build the
  /// functionSurfaces
  int minimum_points(bool constraint_flag) const;

  /// returns the recommended number of samples recommended to build the
  /// functionSurfaces
  int recommended_points(bool constraint_flag) const;

  void active_model_key(const Pecos::ActiveKey& key);
  void clear_model_keys();

  void approximation_function_indices(const SizetSet& approx_fn_indices);

  //void link_multilevel_approximation_data();

  void update_approximation(const Variables& vars,
			    const IntResponsePair& response_pr);
  void update_approximation(const RealMatrix& samples,
			    const IntResponseMap& resp_map);
  void update_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map);

  void append_approximation(const Variables& vars,
			    const IntResponsePair& response_pr);
  void append_approximation(const RealMatrix& samples,
			    const IntResponseMap& resp_map);
  void append_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map);
  void append_approximation(const IntVariablesMap& vars_map,
			    const IntResponseMap&  resp_map);

  void replace_approximation(const IntResponsePair& response_pr);
  void replace_approximation(const IntResponseMap& resp_map);
  void track_evaluation_ids(bool track);

  void build_approximation(const RealVector&  c_l_bnds,
			   const RealVector&  c_u_bnds,
			   const IntVector&  di_l_bnds,
			   const IntVector&  di_u_bnds,
			   const RealVector& dr_l_bnds,
			   const RealVector& dr_u_bnds);

  void export_approximation();

  void rebuild_approximation(const BitArray& rebuild_fns);
  void pop_approximation(bool save_data);
  void push_approximation();
  bool push_available();
  void finalize_approximation();

  void combine_approximation();
  void combined_to_active(bool clear_combined = true);

  void clear_inactive();
  void clear_current_active_data();
  void clear_active_data();

  SharedApproxData& shared_approximation();
  const SizetSet& approximation_fn_indices() const;
  std::vector<Approximation>& approximations();
  const Pecos::SurrogateData& approximation_data(size_t fn_index);

  const RealVectorArray& approximation_coefficients(bool normalized = false);
  void approximation_coefficients(const RealVectorArray& approx_coeffs,
				  bool normalized = false);

  const RealVector& approximation_variances(const Variables& vars);

  void discrepancy_emulation_mode(short mode);

  bool formulation_updated() const;
  void formulation_updated(bool update);

  bool advancement_available();

  Real2DArray cv_diagnostics(const StringArray& metrics, unsigned num_folds);
  Real2DArray challenge_diagnostics(const StringArray& metric_types,
				    const RealMatrix& challenge_pts,
				    const RealVector& challenge_resps);

  // mimic asynchronous operations for those iterators which call
  // asynch_compute_response and synchronize/synchronize_nowait on an
  // approximateModel
  const IntResponseMap& synchronize();
  const IntResponseMap& synchronize_nowait();

  //
  //- Heading: Data
  //

  /// list of approximations, one per response function
  /** This formulation allows the use of mixed approximations (i.e.,
      different approximations used for different response functions),
      although the input specification is not currently general enough
      to support it. */
  std::vector<Approximation> functionSurfaces;

private:

  //
  //- Heading: Convenience functions
  //

  /// add variables/response data to functionSurfaces using a mixture
  /// of shallow and deep copies
  void mixed_add(const Variables& vars, const IntResponsePair& response_pr,
		 bool anchor);
  /// add variables/response data to functionSurfaces using a mixture
  /// of shallow and deep copies
  void mixed_add(const Real* c_vars, const IntResponsePair& response_pr,
		 bool anchor);
  /// add variables/response data to functionSurfaces using a shallow copy
  void shallow_add(const Variables& vars, const IntResponsePair& response_pr,
		   bool anchor);

  /// populate continuous variables within vars from sample_c_vars
  void sample_to_variables(const Real* sample_c_vars, size_t num_cv,
			   Variables& vars);

  /// append to the stack of pop counts within each of the functionSurfaces
  /// based on the active set definition within a single incoming response
  void update_pop_counts(const IntResponsePair& response_pr);
  /// append to the stack of pop counts within each of the functionSurfaces
  /// based on the active set definitions within a map of incoming responses
  void update_pop_counts(const IntResponseMap& resp_map);

  /// helper to find a cached PRP record in data_pairs
  PRPCacheCIter cache_lookup(const Variables& vars, int eval_id,
			     const Response& response);
  /// helper to find a cached PRP record in data_pairs
  PRPCacheCIter cache_lookup(const Real* vars, size_t num_v, int eval_id,
			     const Response& response);
  /// verify consistency between two evaluation identifiers
  void check_id(int id1, int id2);

  /// following Approximation::add() and Approximation::pop_count() operations,
  /// which may enumerate multiple embedded keys, restore the active approxData
  /// to the nominal key
  void restore_data_key();

  /// check on valid indices for singleton active keys
  void check_singleton_key_index(size_t index);
  /// for aggregate active keys that input a single set of data, some
  /// mapping from QoI sets to key indices may be required
  void qoi_set_to_key_index(size_t qoi_set, size_t& key_index);

  /// Load approximation test points from user challenge points file
  void read_challenge_points();

  //
  //- Heading: Data
  //

  /// counter for giving unique names to approximation interfaces
  static size_t approxIdNum;
  /// for incomplete approximation sets, this array specifies the
  /// response function subset that is approximated
  SizetSet approxFnIndices;

  /// data that is shared among all functionSurfaces
  SharedApproxData sharedData;

  /// array of approximation coefficient vectors, one per response function
  RealVectorArray functionSurfaceCoeffs;
  /// vector of approximation variances, one value per response function
  RealVector functionSurfaceVariances;

  // vector of approximation scalings from approx_scale_offset.in.  Provides
  // a capability to reuse existing surrogates with modified design goals.
  //RealVector approxScale;
  // vector of approximation offsets from approx_scale_offset.in.  Provides
  // a capability to reuse existing surrogates with modified design goals.
  //RealVector approxOffset;

  /// flag to activate the tracking of evaluation ids within surrogate data
  bool trackEvalIds;

  //bool graph3DFlag; // controls 3D graphics of approximation surfaces

  /// data file for user-supplied challenge data (per interface, since may
  /// contain multiple responses)
  String challengeFile;
  /// tabular format of the challenge points file
  unsigned short challengeFormat;
  /// whether to validate variable labels in header
  bool challengeUseVarLabels;
  /// whether to import active only
  bool challengeActiveOnly;
  /// container for the challenge points data (variables only)
  RealMatrix challengePoints;
  /// container for the challenge points data (responses only)
  RealMatrix challengeResponses;

  /// copy of the actualModel variables object used to simplify conversion 
  /// among differing variable views
  Variables actualModelVars;
  /// indicates usage of an evaluation cache by the actualModel
  bool actualModelCache;
  /// the interface id from the actualModel used for ordered PRPCache lookups
  String actualModelInterfaceId;

  /// bookkeeping map to catalogue responses generated in map() for use in
  /// synchronize() and synchronize_nowait(). This supports pseudo-asynchronous
  /// operations (approximate responses are always computed synchronously,
  /// but asynchronous virtual functions are supported through bookkeeping).
  IntResponseMap beforeSynchResponseMap;
};


inline ApproximationInterface::~ApproximationInterface() { }


inline Approximation& ApproximationInterface::function_surface(size_t index)
{ return functionSurfaces[index]; }


inline const Approximation& ApproximationInterface::function_surface(size_t index) const
{ return functionSurfaces[index]; }

inline int ApproximationInterface::field_component(size_t fn_index) const
{ return 0; }

inline size_t ApproximationInterface::num_function_surfaces() const
{ return functionSurfaces.size(); }

inline int ApproximationInterface::minimum_points(bool constraint_flag) const
{
  // minimum number of points required over all approximations (even though
  // different approximation types are not yet supported).  Recompute this at
  // the time needed, since it may vary (depending on presence of constraints).
  int min_points = 0;
  for (StSCIter cit=approxFnIndices.begin(); cit!=approxFnIndices.end(); cit++)
    min_points = std::max(min_points,
			  function_surface(*cit).min_points(constraint_flag));
  return min_points;
}


inline int ApproximationInterface::
recommended_points(bool constraint_flag) const
{
  // recommended number of points required over all approximations (even though
  // different approximation types are not yet supported).  Recompute this at
  // the time needed, since it may vary (depending on presence of constraints).
  int rec_points = 0;
  for (StSCIter cit=approxFnIndices.begin(); cit!=approxFnIndices.end(); cit++)
    rec_points = std::max(rec_points, 
      function_surface(*cit).recommended_points(constraint_flag));
  return rec_points;
}


inline void ApproximationInterface::
active_model_key(const Pecos::ActiveKey& key)
{
  sharedData.active_model_key(key);

  // functionSurfaces access active key at run time through shared data; 
  // however they each contain their own approxData which must be updated.
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    function_surface(*it).active_model_key(key);
}


inline void ApproximationInterface::clear_model_keys()
{
  sharedData.clear_model_keys();

  // No Approximation currently requires a default key assignment at construct
  // time: all key assignments can be performed at run time.  However, they
  // each contain their own approxData which must be cleared.
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // Approximation::approxData: clear all keys
    function_surface(*it).clear_model_keys();
}


/** Restore active key in approxData using shared key. */
inline void ApproximationInterface::restore_data_key()
{
  const Pecos::ActiveKey& active_key = sharedData.active_model_key();
  bool reduce_key = (active_key.aggregated() &&
		     active_key.raw_with_reduction_data());
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = function_surface(*it);
    fn_surf.active_model_key(active_key); // reassign aggregate key
    // In addition to restoring the original (aggregate) key, we synchronize
    // the data size for this key with the data size for the embedded keys
    // (which have been enumerated prior to restore_data_key()).  This allows
    // proper use of points() in downstream checks for the top-level key, prior
    // to processing the reductions (e.g., in Pecos::PolynomialApproximation::
    // synchronize_surrogate_data()).
    if (reduce_key) fn_surf.surrogate_data().synchronize_reduction_size();
  }
}


inline void ApproximationInterface::check_singleton_key_index(size_t index)
{
  if (index > 0) {
    Cerr << "Error: invalid index (" << index << ") for singleton "
	 << "active key in ApproximationInterface." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


inline void ApproximationInterface::
qoi_set_to_key_index(size_t qoi_set, size_t& key_index)
{
  // After switching previous {HF,LF} pairings to use a consistent low-to-high
  // ordering for all ensembles, the shortcut enumeration of key_index using
  // the QoI set index was no longer sufficient for all cases.

  // In particular, we need to manage unusual combinations resulting from use
  // of synthetic data, i.e. for recursive surrogate emulation.  
  // > aggregate key for {synthetic LF, simulation HF} + discrepancy reduction,
  //   combined with a responseMode for adding simulation data only
  //   --> need key index = 1 (HF) for incoming qoi_set index = 0 (one QoI set)
  // > All other current use cases are identity mappings

  // Options:
  // > could return to {HF,LF} ordering --> No, just flips the logic gap to an
  //   unused combination --> would have bad indexing with {HF,LF} pairs for
  //   SURROGATE modes w/i aggregated key (not currently used; no synthetic HF)
  // > Alter key management logic: extract the approximation data targets for
  //   {mixed,shallow}_add() from activeKey data based on modes
  // > Simpler: just map qoi_set to key_index using mode information (accesses
  //   the desired embedded key without needing to cache it separately).

  size_t num_key_data = sharedData.active_model_key().data_size();
  if (num_key_data > 1) {

    /* Logic based on surrogate response mode:
    // > AGGREGATED modes: key_index = qoi_set
    // > SURROGATE modes:  key_index = qoi_set = 0
    // > TRUTH modes + aggregated key (synth data): qoi_set = 0 --> key = last
    // Issue is advanced MLMF response modes are managed within iteratedModel
    // (the EnsembleSurrModel), not uSpaceModel (the DataFitSurrModel)
    //
    switch (sharedData.surrogate_response_mode()) {
    case BYPASS_SURROGATE:  case NO_SURROGATE:
      check_singleton_key_index(qoi_set);  // incoming should be zero
      key_index = num_key_data -1;// - qoi_set;// reverse indexing (maps 0 to 1)
      break;
    case UNCORRECTED_SURROGATE:  case AUTO_CORRECTED_SURROGATE:
    case MODEL_DISCREPANCY:
      check_singleton_key_index(qoi_set); // incoming should be zero
      key_index = 0;        break;
    default: // identity map for AGGREGATED modes
      key_index = qoi_set;  break;
    }
    */

    // Logic based on discrepancy mode: access embedded truth key if recursive
    switch (sharedData.discrepancy_emulation_mode()) {
    case RECURSIVE_EMULATION: {
      check_singleton_key_index(qoi_set); // incoming should be zero
      size_t last_index = num_key_data - 1;
      key_index = last_index - qoi_set; // reverse indexing (maps 0->1 for now)
      // Note: reduction data is assigned to aggregate key
      // (embedded keys track "raw" data sources)
      break;
    }
    default: // DISTINCT_EMULATION
      key_index = qoi_set;  break;
    }

  }
  else {
    check_singleton_key_index(qoi_set);
    key_index = qoi_set;
  }
}


inline void ApproximationInterface::discrepancy_emulation_mode(short mode)
{ sharedData.discrepancy_emulation_mode(mode); }


inline void ApproximationInterface::
approximation_function_indices(const SizetSet& approx_fn_indices)
{ approxFnIndices = approx_fn_indices; }


/*
inline void ApproximationInterface::link_multilevel_approximation_data()
{
  // define approx data keys and active index
  sharedData.link_multilevel_surrogate_data();

  // (create and) link SurrogateData instances
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    function_surface(*it).link_multilevel_surrogate_data();
}
*/


inline void ApproximationInterface::track_evaluation_ids(bool track)
{ trackEvalIds = track; }


/** This function removes data provided by a previous append_approximation()
    call, possibly different numbers for each function, or as specified in
    pop_count, which is assumed to be the same for all functions. */
inline void ApproximationInterface::pop_approximation(bool save_data)
{
  sharedData.pop(save_data); // operation order not currently important

  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = function_surface(*it);
    // Approximation::approxData (pop for top and potentially embedded keys)
    fn_surf.pop_data(save_data);
    // Approximation coefficients
    fn_surf.pop_coefficients(save_data);
  }
}


/** This function updates the coefficients for each Approximation based
    on data increments provided by {update,append}_approximation(). */
inline void ApproximationInterface::push_approximation()
{
  sharedData.pre_push(); // do shared aggregation first

  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = function_surface(*it);
    // Approximation::approxData (push for top and potentially embedded keys)
    fn_surf.push_data(); // uses shared restoration index
    // Approximation coefficients
    fn_surf.push_coefficients();
  }

  sharedData.post_push(); // do shared cleanup last
}


inline bool ApproximationInterface::push_available()
{ return sharedData.push_available(); }


inline void ApproximationInterface::finalize_approximation()
{
  sharedData.pre_finalize(); // do shared aggregation first

  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = function_surface(*it);
    // Approximation::approxData (finalize for top & potentially embedded keys)
    fn_surf.finalize_data(); // uses shared finalization index
    // Approximation coefficients
    fn_surf.finalize_coefficients();
  }

  sharedData.post_finalize(); // do shared cleanup last
}


inline void ApproximationInterface::combine_approximation()
{
  sharedData.pre_combine(); // shared aggregation first

  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    function_surface(*it).combine_coefficients();

  sharedData.post_combine(); // shared cleanup last
}


inline void ApproximationInterface::combined_to_active(bool clear_combined)
{
  sharedData.combined_to_active(clear_combined); // shared aggregation first

  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    function_surface(*it).combined_to_active_coefficients(clear_combined);
}


inline void ApproximationInterface::clear_inactive()
{
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = function_surface(*it);
    // Approximation::approxData: only retain 1st of active data keys
    fn_surf.clear_inactive_data();
    // Approximation coefficients
    fn_surf.clear_inactive_coefficients();
  }

  sharedData.clear_inactive(); // shared cleanup last
}


inline void ApproximationInterface::clear_current_active_data()
{
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); it++)
    function_surface(*it).clear_current_active_data();
}


inline void ApproximationInterface::clear_active_data()
{
  for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); it++)
    function_surface(*it).clear_active_data();
}


inline bool ApproximationInterface::formulation_updated() const
{ return sharedData.formulation_updated(); }


inline void ApproximationInterface::formulation_updated(bool update)
{ return sharedData.formulation_updated(update); }


inline bool ApproximationInterface::advancement_available()
{
  // this logic assesses whether there is headroom for rank/order advancement

  // Note: once rank/order advanced by SharedApproxData::increment_order(),
  // DataFitSurrModel::rebuild_global() rebuilds for either a numSamples
  // increment or if approxInterface.formulation_updated() for fixed data
  // (e.g., for an advanced bound that could admit a different adapted soln)

  if (sharedData.advancement_available()) return true; // check Shared first
  else {
    bool refine = false;
    for (StSIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
      if (function_surface(*it).advancement_available())
	refine = true; // no break; accumulate advancement types across fns
    return refine;
  }
}


inline const SizetSet& ApproximationInterface::approximation_fn_indices() const
{ return approxFnIndices; }


inline SharedApproxData& ApproximationInterface::shared_approximation()
{ return sharedData; }


inline std::vector<Approximation>& ApproximationInterface::approximations()
{ return functionSurfaces; }


inline const Pecos::SurrogateData& ApproximationInterface::
approximation_data(size_t fn_index)
{
  if (approxFnIndices.find(fn_index) == approxFnIndices.end()) {
    Cerr << "Error: index passed to ApproximationInterface::approximation_data"
	 << "() does not correspond to an approximated function." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  return function_surface(fn_index).surrogate_data();
}


inline void ApproximationInterface::
sample_to_variables(const Real* sample_c_vars, size_t num_cv, Variables& vars)
{
  if (vars.cv() == num_cv)
    for (size_t i=0; i<num_cv; ++i)
      vars.continuous_variable(sample_c_vars[i], i);
  else if (vars.acv() == num_cv)
    for (size_t i=0; i<num_cv; ++i)
      vars.all_continuous_variable(sample_c_vars[i], i);
  else {
    Cerr << "Error: size mismatch in ApproximationInterface::"
	 << "sample_to_variables()" << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


inline void ApproximationInterface::check_id(int id1, int id2)
{
  if (id1 != id2) {
    Cerr << "Error: id mismatch in ApproximationInterface::check_id()"
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

} // namespace Dakota

#endif
