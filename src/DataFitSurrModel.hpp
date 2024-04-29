/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DATA_FIT_SURR_MODEL_H
#define DATA_FIT_SURR_MODEL_H

#include "dakota_data_types.hpp"
#include "SurrogateData.hpp"
#include "SurrogateModel.hpp"
#include "DakotaInterface.hpp"
#include "DakotaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"


namespace Dakota {

/// Derived model class within the surrogate model branch for managing
/// data fit surrogates (global and local)

/** The DataFitSurrModel class manages global or local approximations
    (surrogates that involve data fits) that are used in place of an
    expensive model.  The class contains an approxInterface (required
    for both global and local) which manages the approximate function
    evaluations, an actualModel (optional for global, required for
    local) which provides truth evaluations for building the
    surrogate, and a daceIterator (optional for global, not used for
    local) which selects parameter sets on which to evaluate
    actualModel in order to generate the necessary data for building
    global approximations. */

class DataFitSurrModel: public SurrogateModel
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  DataFitSurrModel(ProblemDescDB& problem_db);
  /// alternate constructor for instantiations on the fly
  DataFitSurrModel(Iterator& dace_iterator, Model& actual_model,
		   const ActiveSet& dfs_set, const ShortShortPair& dfs_view,
		   const String& approx_type, const UShortArray& approx_order,
		   short corr_type, short corr_order, short data_order,
		   short output_level, const String& point_reuse,
		   const String& import_build_points_file = String(),
		   unsigned short import_build_format = TABULAR_ANNOTATED,
		   bool import_build_active_only = false,
		   const String& export_approx_points_file = String(),
		   unsigned short export_approx_format = TABULAR_ANNOTATED);
  /// destructor
  ~DataFitSurrModel();

  //
  //- Heading: Member functions
  //

  /// set pointsTotal and pointsManagement mode
  void total_points(int points);
  /// return points required for build according to pointsManagement mode
  int required_points();

  void declare_sources();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  size_t qoi() const;

  DiscrepancyCorrection& discrepancy_correction();
  void correction_type(short corr_type);

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();

  //void init_model(Model& model);
  void update_model(Model& model);
  void update_from_model(const Model& model);

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

  // Perform the response computation portions specific to this derived 
  // class.  In this case, it simply employs approxInterface.map()/synch()/
  // synch_nowait() where approxInterface is a local, multipoint, or global
  // approximation.

  void derived_evaluate(const ActiveSet& set);
  void derived_evaluate_nowait(const ActiveSet& set);
  const IntResponseMap& derived_synchronize();
  const IntResponseMap& derived_synchronize_nowait();

  /// map incoming ASV into actual request for surrogate construction, managing
  /// any mismatch in sizes due to response aggregation modes in actualModel
  void asv_inflate_build(const ShortArray& orig_asv, ShortArray& actual_asv);
  /// split incoming ASV into actual and approximate evaluation requests,
  /// managing any mismatch in sizes due to response aggregation modes in
  /// actualModel
  void asv_split(const ShortArray& orig_asv, ShortArray& actual_asv,
		 ShortArray& approx_asv);

  /// return daceIterator
  Iterator& subordinate_iterator();

  /// set active model key within approxInterface
  void active_model_key(const Pecos::ActiveKey& key);
  /// remove all model keys within approxInterface
  void clear_model_keys();

  /// return this model instance
  Model& surrogate_model(size_t i = _NPOS);
  /// return this model instance
  const Model& surrogate_model(size_t i = _NPOS) const;
  /// return actualModel
  Model& truth_model();
  /// return actualModel
  const Model& truth_model() const;

  /// return actualModel (and optionally its sub-models)
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);
  /// pass request to actualModel if recursing
  void resize_from_subordinate_model(size_t depth = SZ_MAX);
  /// pass request to actualModel if recursing and then update from it
  void update_from_subordinate_model(size_t depth = SZ_MAX);
  /// return approxInterface
  Interface& derived_interface();

  /// set the relative weightings for multiple objective functions or least
  /// squares terms and optionally recurses into actualModel
  void primary_response_fn_weights(const RealVector& wts,
				   bool recurse_flag = true);

  /// set responseMode and pass any bypass request on to actualModel for
  /// any lower-level surrogates.
  void surrogate_response_mode(short mode);
  /// set approxInterface.sharedData.discrepEmulationMode
  void discrepancy_emulation_mode(short mode);

  // link together more than one SurrogateData instance within
  // approxInterface.functionSurfaces[i].approxData[j]
  //void link_multilevel_approximation_data();

  /// (re)set the surrogate index set in SurrogateModel::surrogateFnIndices
  /// and ApproximationInterface::approxFnIndices
  void surrogate_function_indices(const SizetSet& surr_fn_indices);

  bool force_rebuild();

  /// Builds the local/multipoint/global approximation using
  /// daceIterator/actualModel to generate new data points
  void build_approximation();
  /// Builds the local/multipoint/global approximation using
  /// daceIterator/actualModel to generate new data points that
  /// augment the passed vars/response anchor point
  bool build_approximation(const Variables& vars,
			   const IntResponsePair& response_pr);

  /// Rebuilds the local/multipoint/global approximation using
  /// daceIterator/actualModel to generate an increment of appended data
  void rebuild_approximation();
  /// Rebuilds the local/multipoint/global approximation using
  /// the passed response data for a single sample
  void rebuild_approximation(const IntResponsePair& response_pr);
  /// Rebuilds the local/multipoint/global approximation using
  /// the passed response data for a set of samples
  void rebuild_approximation(const IntResponseMap& resp_map);

  /// replaces the approximation data with daceIterator results and
  /// rebuilds the approximation if requested
  void update_approximation(bool rebuild_flag);
  /// replaces the anchor point, and rebuilds the approximation if requested
  void update_approximation(const Variables& vars,
			    const IntResponsePair& response_pr,
			    bool rebuild_flag);
  /// replaces the current points array and rebuilds the approximation
  /// if requested
  void update_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map, bool rebuild_flag);
  /// replaces the current points array and rebuilds the approximation
  /// if requested
  void update_approximation(const RealMatrix& samples,
			    const IntResponseMap& resp_map, bool rebuild_flag);

  /// appends daceIterator results to a global approximation and rebuilds
  /// it if requested
  void append_approximation(bool rebuild_flag);
  /// appends a point to a global approximation and rebuilds it if requested
  void append_approximation(const Variables& vars,
			    const IntResponsePair& response_pr,
			    bool rebuild_flag);
  /// appends a matrix of points to a global approximation and rebuilds it
  /// if requested
  void append_approximation(const RealMatrix& samples,
			    const IntResponseMap& resp_map, bool rebuild_flag);
  /// appends an array of points to a global approximation and rebuilds it
  /// if requested
  void append_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map, bool rebuild_flag);
  /// appends an map of points to a global approximation and rebuilds it
  /// if requested
  void append_approximation(const IntVariablesMap& vars_map,
			    const IntResponseMap&  resp_map, bool rebuild_flag);

  void replace_approximation(const IntResponsePair& response_pr,
			     bool rebuild_flag);
  void replace_approximation(const IntResponseMap& resp_map, bool rebuild_flag);
  void track_evaluation_ids(bool track);

  void pop_approximation(bool save_surr_data, bool rebuild_flag = false);

  void push_approximation();
  bool push_available();

  void finalize_approximation();

  /// combine all level approximations into a separate composite approximation
  void combine_approximation();
  /// promote the combined approximation into the active one
  void combined_to_active(bool clear_combined = true);

  /// clear inactive data stored in the approxInterface
  void clear_inactive();

  /// query approxInterface for available advancements in order, rank, etc.
  bool advancement_available();
  /// query approxInterface for updates in formulation (requiring a rebuild)
  bool formulation_updated() const;
  /// update the formulation status in approxInterface
  void formulation_updated(bool update);
  
  /// execute the DACE iterator to generate build data
  void run_dace();

  /// retrieve the SharedApproxData from approxInterface
  SharedApproxData& shared_approximation();
  /// retrieve the set of Approximations from approxInterface
  std::vector<Approximation>& approximations();
  /// return the approximation coefficients from each Approximation
  /// (request forwarded to approxInterface)
  const RealVectorArray& approximation_coefficients(bool normalized = false);
  /// set the approximation coefficients within each Approximation
  /// (request forwarded to approxInterface)
  void approximation_coefficients(const RealVectorArray& approx_coeffs,
				  bool normalized = false);
  /// return the approximation variance from each Approximation
  /// (request forwarded to approxInterface)
  const RealVector& approximation_variances(const Variables& vars);
  /// return the approximation data from a particular Approximation
  /// (request forwarded to approxInterface)
  const Pecos::SurrogateData& approximation_data(size_t fn_index);

  /// update component parallel mode for supporting parallelism in actualModel
  void component_parallel_mode(short mode);

  IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// set up actualModel for parallel operations
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);
  /// set up actualModel for serial operations.
  void derived_init_serial();
  /// set active parallel configuration within actualModel
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the DataFitSurrModel
  /// (request forwarded to actualModel)
  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);

  /// Service actualModel job requests received from the master.
  /// Completes when a termination message is received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);
  /// Executed by the master to terminate actualModel server operations
  /// when DataFitSurrModel iteration is complete.
  void stop_servers();

  /// update the Model's active view based on higher level context
  /// and optionally recurse into actualModel
  void active_view(short view, bool recurse_flag = true);
  /// update the Model's inactive view based on higher level context
  /// and optionally recurse into actualModel
  void inactive_view(short view, bool recurse_flag = true);

  /// return the approxInterface identifier
  const String& interface_id() const;
  /// if recurse_flag, return the actualModel evaluation cache usage
  bool evaluation_cache(bool recurse_flag = true) const;
  /// if recurse_flag, return the actualModel restart file usage
  bool restart_file(bool recurse_flag = true) const;

  /// set the evaluation counter reference points for the DataFitSurrModel
  /// (request forwarded to approxInterface and actualModel)
  void set_evaluation_reference();
  /// request fine-grained evaluation reporting within approxInterface
  /// and actualModel
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the DataFitSurrModel
  /// (request forwarded to approxInterface and actualModel)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const;

  /// set the warm start flag, including actualModel
  void warm_start_flag(const bool flag);

  ActiveSet default_interface_active_set();

  //
  //- Heading: Data members
  //

  /// whether to export the surrogate to file
  const bool exportSurrogate;

  /// whether to automatically refine the surrogate during the build phase
  const bool autoRefine;
  /// Maximum number of times to refine the surrogate
  const size_t maxIterations;
  /// Maximum number of evaluations while refining a surrogate
  const size_t maxFuncEvals;
  /// Convergence criterion, compared to CV score for specified metric
  const Real convergenceTolerance;
  /// Max number of iterations for which there is no average improvement
  const int softConvergenceLimit;
  /// Type of error metric to test for surrogate refinement convegence 
  const String refineCVMetric;
  /// Number of cross validation folds for surrogate refinement
  const int refineCVFolds;

private:

  //
  //- Heading: Convenience functions
  //

  /// optionally read surrogate data points from provided file
  void import_points(unsigned short tabular_format, bool use_var_labels,
		     bool active_only);
  /// initialize file stream for exporting surrogate evaluations
  void initialize_export();
  /// finalize file stream for exporting surrogate evaluations
  void finalize_export();
  /// initialize file stream for exporting surrogate evaluations
  void export_point(int eval_id, const Variables& vars, const Response& resp);

  /// Common code for processing of approximate response maps shared by
  /// derived_synchronize() and derived_synchronize_nowait()
  void derived_synchronize_approx(bool block,
				  IntResponseMap& approx_resp_map_rekey);

  /// Updates fit arrays for local or multipoint approximations
  void update_local_reference();
  /// Builds a local or multipoint approximation using actualModel
  void build_local_multipoint();
  /// Builds a local or multipoint approximation using actualModel
  void build_local_multipoint(const Variables& vars,
			      const IntResponsePair& response_pr);

  /// Updates fit arrays for global approximations
  void update_global_reference();
  /// Builds a global approximation using daceIterator
  void build_global();
  /// Rebuilds a global approximation by generating new data using
  /// daceIterator and appending to approxInterface
  void rebuild_global();

  /// Refine the built surrogate until convergence criteria are met
  void refine_surrogate();

  /// clear current data from approxInterface
  void clear_approx_interface();
  /// update anchor data in approxInterface
  void update_approx_interface(const Variables& vars,
			       const IntResponsePair& response_pr);
  /// build the approxInterface surrogate, passing variable bounds
  void build_approx_interface();

  /// test if inactive state is consistent
  bool consistent(const Variables& vars) const;
  /// test if active vars are within [l_bnds, u_bnds]
  bool inside(const Variables& vars) const;
  /// test for exact equality in values between active vars and sdv
  bool active_vars_compare(const Variables& vars,
			   const Pecos::SurrogateDataVars& sdv) const;

  //
  //- Heading: Data members
  //

  /// manages the building and subsequent evaluation of the approximations
  /// (required for both global and local)
  Interface approxInterface;
  /// the truth model which provides evaluations for building the surrogate
  /// (optional for global, required for local)
  /** actualModel is unrestricted in type; arbitrary nestings are possible. */
  Model actualModel;
  /// selects parameter sets on which to evaluate actualModel in order
  /// to generate the necessary data for building global approximations
  /// (optional for global since restart data may also be used)
  Iterator daceIterator;

  /// manages construction and application of correction functions that
  /// are applied to a surrogate model (DataFitSurr or EnsembleSurr) in
  /// order to reproduce high fidelity data.
  DiscrepancyCorrection deltaCorr;

  // Note: the following Maps are a simpler case of the more general
  // {modelId,cachedResp}Maps in EnsembleSurrModel (consolidating would
  // add overhead for DFSModel).

  /// map from actualModel/highFidelityModel evaluation ids to
  /// DataFitSurrModel/EnsembleSurrModel ids
  IntIntMap truthIdMap;
  /// map from approxInterface/lowFidelityModel evaluation ids to
  /// DataFitSurrModel/EnsembleSurrModel ids
  IntIntMap surrIdMap;
  /// map of approximate responses retrieved in derived_synchronize_nowait()
  /// that could not be returned since corresponding truth model response
  /// portions were still pending.
  IntResponseMap cachedApproxRespMap;

  /// total points the user specified to construct the surrogate
  int pointsTotal;
  /// configuration for points management in build_global()
  short pointsManagement;

  /// type of point reuse for approximation builds: \c all, \c region
  /// (default if points file), or \c none (default if no points file)
  String pointReuse;
  /// file name from \c import_build_points_file specification
  String importPointsFile;

  /// file name from \c export_approx_points_file specification
  String exportPointsFile;
  /// file export format for variables and approximate responses
  unsigned short exportFormat;
  /// output file stream for \c export_approx_points_file specification
  std::ofstream exportFileStream;

  /// file name from \c export_approx_variance_file specification
  String exportVarianceFile;
  /// file export format for variables and approximate response variance
  unsigned short exportVarianceFormat;
  /// output file stream for \c export_approx_variance_file specification
  std::ofstream exportVarianceFileStream;

  /// stores a reference copy of the inactive continuous variables when the
  /// approximation is built using a Distinct view; used to detect when a
  /// rebuild is required.
  RealVector referenceICVars;
  /// stores a reference copy of the inactive discrete int variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  IntVector referenceIDIVars;
  /// stores a reference copy of the inactive discrete string variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  StringMultiArray referenceIDSVars;
  /// stores a reference copy of the inactive discrete real variables when
  /// the approximation is built using a Distinct view; used to detect when
  /// a rebuild is required.
  RealVector referenceIDRVars;

  /// stores a reference copy of active continuous lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceCLBnds;
  /// stores a reference copy of active continuous upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceCUBnds;
  /// stores a reference copy of active discrete int lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  IntVector referenceDILBnds;
  /// stores a reference copy of active discrete int upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  IntVector referenceDIUBnds;
  /// stores a reference copy of active discrete real lower bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceDRLBnds;
  /// stores a reference copy of active discrete real upper bounds when the
  /// approximation is built; used to detect when a rebuild is required.
  RealVector referenceDRUBnds;
};


inline DataFitSurrModel::~DataFitSurrModel()
{ if (!exportPointsFile.empty()) finalize_export(); }


inline void DataFitSurrModel::
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
  if (!actualModel.is_null())
    actualModel.nested_variable_mappings(c_index1, di_index1, ds_index1,
					 dr_index1, c_target2, di_target2,
					 ds_target2, dr_target2);
}


inline const SizetArray& DataFitSurrModel::nested_acv1_indices() const
{ return actualModel.nested_acv1_indices(); }


inline const ShortArray& DataFitSurrModel::nested_acv2_targets() const
{ return actualModel.nested_acv2_targets(); }


inline short DataFitSurrModel::query_distribution_parameter_derivatives() const
{
  return (actualModel.is_null()) ? NO_DERIVS :
    actualModel.query_distribution_parameter_derivatives(); // forward along
}


inline bool DataFitSurrModel::force_rebuild()
{
  return check_rebuild(referenceICVars,  referenceIDIVars, referenceIDSVars,
		       referenceIDRVars, referenceCLBnds,  referenceCUBnds,
		       referenceDILBnds, referenceDIUBnds, referenceDRLBnds,
		       referenceDRUBnds);
}


inline size_t DataFitSurrModel::qoi() const
{
  // Response inflation from aggregation does not proliferate above
  // this Model recursion level
  return (responseMode == AGGREGATED_MODEL_PAIR && !actualModel.is_null()) ?
    actualModel.qoi() : response_size();

  //switch (responseMode) {
  //case AGGREGATED_MODEL_PAIR:
  //  if (actualModel.is_null()) return response_size();
  //  else                       return actualModel.qoi();
  //  break;
  //default:                     return response_size();    break;
  //}
}


inline DiscrepancyCorrection& DataFitSurrModel::discrepancy_correction()
{ return deltaCorr; }


inline void DataFitSurrModel::correction_type(short corr_type)
{ corrType = corr_type; deltaCorr.correction_type(corr_type); }


inline void DataFitSurrModel::total_points(int points)
{ pointsTotal = points; if (points > 0) pointsManagement = TOTAL_POINTS; }


inline int DataFitSurrModel::required_points()
{
  switch (pointsManagement) {
  case TOTAL_POINTS: {
    int min_points = approxInterface.minimum_points(true);
    if (pointsTotal < min_points && outputLevel >= NORMAL_OUTPUT)
      Cout << "\nDataFitSurrModel: Total points specified (" << pointsTotal
	   << ") is less than minimum required;\n                  "
	   << "increasing to " << min_points << std::endl;
    return std::max(min_points, pointsTotal);        break;
  }
  case RECOMMENDED_POINTS:
    return approxInterface.recommended_points(true); break;
  default: //case DEFAULT_POINTS: case MINIMUM_POINTS:
    return approxInterface.minimum_points(true);     break;
  }
}


inline bool DataFitSurrModel::
active_vars_compare(const Variables& vars,
		    const Pecos::SurrogateDataVars& sdv) const
{
  // Similar to id_vars_exact_compare() in PRPMultiIndex.hpp

  if (vars.is_null() || sdv.is_null())
    return false;
  // discrete strings not currently included in SurrogateDataVars
  else if (vars.continuous_variables()    != sdv.continuous_variables() ||
	   vars.discrete_int_variables()  != sdv.discrete_int_variables() ||
	   vars.discrete_real_variables() != sdv.discrete_real_variables())
    return false;

  return true;
}


inline Iterator& DataFitSurrModel::subordinate_iterator()
{ return daceIterator; }


inline void DataFitSurrModel::active_model_key(const Pecos::ActiveKey& key)
{
  // assign activeKey
  SurrogateModel::active_model_key(key);

  // recur both components: (actualModel could be hierarchical)
  approxInterface.active_model_key(key);
  if (!actualModel.is_null()) actualModel.active_model_key(key);
}


inline void DataFitSurrModel::clear_model_keys()
{ approxInterface.clear_model_keys(); }


inline void DataFitSurrModel::discrepancy_emulation_mode(short mode)
{ approxInterface.discrepancy_emulation_mode(mode); }


inline Model& DataFitSurrModel::surrogate_model(size_t i)
{
  if (i && i != _NPOS) { // allow either 0 or no index (defaults to _NPOS)
    Cerr << "Error: bad index (" << i << ") in DataFitSurrModel::"
	 << "surrogate_model()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return *this; // return by reference: OK to return letter instance

  // return by value: letter instance must be returned within an envelope for
  // representation sharing/reference counting to work properly
  //Model surr_model;            // empty envelope
  //surr_model.assign_rep(this); // populate letter, increment reference count
  //return surr_model;
}


inline const Model& DataFitSurrModel::surrogate_model(size_t i) const
{
  if (i && i != _NPOS) { // allow either 0 or no index (defaults to _NPOS)
    Cerr << "Error: bad index (" << i << ") in DataFitSurrModel::"
	 << "surrogate_model()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return *this; // return of letter (see above)
}


inline Model& DataFitSurrModel::truth_model()
{ return actualModel; }


inline const Model& DataFitSurrModel::truth_model() const
{ return actualModel; }


inline void DataFitSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  if (!actualModel.is_null()) {
    ml.push_back(actualModel);
    if (recurse_flag)
      actualModel.derived_subordinate_models(ml, recurse_flag);
  }
}


inline void DataFitSurrModel::resize_from_subordinate_model(size_t depth)
{
  if (!actualModel.is_null() && depth) {
    // data flows from the bottom-up, so recurse first
    if (depth == SZ_MAX)
      actualModel.resize_from_subordinate_model(depth); // retain special value
    else
      actualModel.resize_from_subordinate_model(depth - 1);

    // DataFitSurrModel consumes (newly) aggregated data sets through multiple
    // SurrogateData instances --> don't resize locally.
    //resize_response();
    // It must therefore manage inflation of incoming ActiveSet instances

    // daceIterator::activeSet muse be resized for consistency with actualModel
    if (!daceIterator.is_null()) {
      const ActiveSet&  dace_set = daceIterator.active_set();
      const ShortArray& dace_asv = dace_set.request_vector();
      size_t num_am_resp = actualModel.response_size(),
	   num_dace_resp = dace_asv.size();
      if (num_am_resp != num_dace_resp) {
	ActiveSet new_set(dace_set); // deep copy
	new_set.reshape(num_am_resp);
	daceIterator.active_set(new_set);
      }
    }
  }
}


inline void DataFitSurrModel::update_from_subordinate_model(size_t depth)
{
  if (!actualModel.is_null()) {
    // data flows from the bottom-up, so recurse first
    if (depth == SZ_MAX)
      actualModel.update_from_subordinate_model(depth); // retain special value
    else if (depth)
      actualModel.update_from_subordinate_model(depth - 1);
    // now pull the latest updates from actualModel
    update_from_model(actualModel);
  }
}


inline Interface& DataFitSurrModel::derived_interface()
{ return approxInterface; }


inline void DataFitSurrModel::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  primaryRespFnWts = wts;
  if (recurse_flag && !actualModel.is_null())
    actualModel.primary_response_fn_weights(wts, recurse_flag);
}


inline void DataFitSurrModel::surrogate_response_mode(short mode)
{
  responseMode = mode;

  // Mode-specific logic:
  // > Compared to EnsembleSurrModel, don't need to be as strict in validating
  //   AUTO_CORRECTED_SURROGATE mode against corrType, since NO_CORRECTION is
  //   an admissible option in the case of global data fits.  However,
  //   MODEL_DISCREPANCY still needs a discrepancy formulation (additive, etc.).
  // > Management of multiple SurrogateData instances is complicated in the
  //   heterogeneous setting where level 0 uses a single instance and levels
  //   1-L use two instances.  In the future, could manage activation explicitly
  //   using functions shown below.  For now, SurrogateData::{push,pop}() are
  //   hardened for inactive instances.
  // > As noted in resize_from_subordinate_model(), aggregated data sets are
  //   consumed by DFSModel so we do not resize the response at this level.
  switch (mode) {
  case MODEL_DISCREPANCY:
    if (!corrType) {
      Cerr << "Error: activation of mode MODEL_DISCREPANCY requires "
	   << "specification of a correction type." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    break;
  case BYPASS_SURROGATE:
    if (actualModel.is_null()) {
      Cerr << "Error: actualModel must be defined for mode BYPASS_SURROGATE."
	   << std::endl;
      abort_handler(MODEL_ERROR);
    }
    actualModel.surrogate_response_mode(mode); // recurse in this case
    //approxInterface.deactivate_multilevel_approximation_data();
    break;
  case AGGREGATED_MODEL_PAIR:
    //approxInterface.activate_multilevel_approximation_data();
    break;
  }
}


//inline void DataFitSurrModel::link_multilevel_approximation_data()
//{ approxInterface.link_multilevel_approximation_data(); }


inline void DataFitSurrModel::
surrogate_function_indices(const SizetSet& surr_fn_indices)
{
  surrogateFnIndices = surr_fn_indices;
  approxInterface.approximation_function_indices(surr_fn_indices);
}


inline bool DataFitSurrModel::push_available()
{ return approxInterface.push_available(); }


inline void DataFitSurrModel::clear_inactive()
{ approxInterface.clear_inactive(); }


inline bool DataFitSurrModel::advancement_available()
{ return approxInterface.advancement_available(); }


inline bool DataFitSurrModel::formulation_updated() const
{ return approxInterface.formulation_updated(); }


inline void DataFitSurrModel::formulation_updated(bool update)
{ approxInterface.formulation_updated(update); }


inline SharedApproxData& DataFitSurrModel::shared_approximation()
{ return approxInterface.shared_approximation(); }


inline std::vector<Approximation>& DataFitSurrModel::approximations()
{ return approxInterface.approximations(); }


inline const RealVectorArray& DataFitSurrModel::
approximation_coefficients(bool normalized)
{ return approxInterface.approximation_coefficients(normalized); }


inline void DataFitSurrModel::
approximation_coefficients(const RealVectorArray& approx_coeffs,
			   bool normalized)
{
  approxInterface.approximation_coefficients(approx_coeffs, normalized);

  // Surrogate data is being imported.  Update state to suppress automatic
  // surrogate construction.
  ++approxBuilds;
  if (strbegins(surrogateType, "global_")) update_global_reference();
  else                                      update_local_reference();
}


inline void DataFitSurrModel::
rebuild_approximation(const IntResponsePair& response_pr)
{
  // decide which surrogates to rebuild based on resp_map content
  BitArray rebuild_fns(numFns); // init to false
  const ShortArray& asv = response_pr.second.active_set_request_vector();
  for (size_t i=0; i<numFns; ++i)
    if (asv[i])
      rebuild_fns.set(i);
  // rebuild the designated surrogates
  approxInterface.rebuild_approximation(rebuild_fns);
  ++approxBuilds;
}


inline void DataFitSurrModel::
rebuild_approximation(const IntResponseMap& resp_map)
{
  // decide which surrogates to rebuild based on resp_map content
  BitArray rebuild_fns(numFns); // init to false
  for (size_t i=0; i<numFns; ++i)
    for (IntRespMCIter r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it)
      if (r_it->second.active_set_request_vector()[i])
	{ rebuild_fns.set(i); break; }
  // rebuild the designated surrogates
  approxInterface.rebuild_approximation(rebuild_fns);
  ++approxBuilds;
}


inline void DataFitSurrModel::track_evaluation_ids(bool track)
{ approxInterface.track_evaluation_ids(track); }


inline const RealVector& DataFitSurrModel::
approximation_variances(const Variables& vars)
{ return approxInterface.approximation_variances(vars); }


inline const Pecos::SurrogateData& DataFitSurrModel::
approximation_data(size_t fn_index)
{ return approxInterface.approximation_data(fn_index); }


inline IntIntPair DataFitSurrModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // support DB-based and on-the-fly instantiations for DataFitSurrModel
  if (!daceIterator.is_null()) {
    probDescDB.set_db_list_nodes(daceIterator.method_id());
    return daceIterator.estimate_partition_bounds();
  }
  else if (!actualModel.is_null()) {
    int am_max_conc = approxInterface.minimum_points(false)
                    * actualModel.derivative_concurrency(); // local/multipt
    probDescDB.set_db_model_nodes(actualModel.model_id());
    return actualModel.estimate_partition_bounds(am_max_conc);
  }
  else
    return IntIntPair(1, 1);
}


inline void DataFitSurrModel::derived_init_serial()
{
  //approxInterface.init_serial();

  if (!actualModel.is_null())
    actualModel.init_serial();
}


inline void DataFitSurrModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  //parallelLib.parallel_configuration_iterator(modelPCIter);
  //approxInterface.set_communicators(messageLengths);
  // asynchEvalFlag and evaluationCapacity updates not required for DFS
  // (refer to {Recast,EnsembleSurr}Model::derived_set_communicators())
  //set_ie_asynchronous_mode(max_eval_concurrency);

  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  if (recurse_flag) {
    if (!daceIterator.is_null())
      daceIterator.set_communicators(pl_iter);
    else if (!actualModel.is_null())
      actualModel.init_communicators(pl_iter,
	daceIterator.maximum_evaluation_concurrency()); // set in init_comms
  }
}


inline void DataFitSurrModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  //parallelLib.parallel_configuration_iterator(modelPCIter);
  //approxInterface.free_communicators();

  if (recurse_flag) {
    if (!daceIterator.is_null())
      daceIterator.free_communicators(pl_iter);
    else if (!actualModel.is_null())
      actualModel.free_communicators(pl_iter,
	daceIterator.maximum_evaluation_concurrency()); // set in init_comms
  }
}


inline void DataFitSurrModel::
serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  // don't recurse, as actualModel.serve() will set actualModel comms
  set_communicators(pl_iter, max_eval_concurrency, false);

  if (!actualModel.is_null())
    actualModel.serve_run(pl_iter,
			  daceIterator.maximum_evaluation_concurrency());
}


inline void DataFitSurrModel::stop_servers()
{
  if (!actualModel.is_null())
    actualModel.stop_servers();
}


inline void DataFitSurrModel::active_view(short view, bool recurse_flag)
{
  Model::active_view(view);
  if (recurse_flag && !actualModel.is_null())
    actualModel.active_view(view, recurse_flag);
}


inline void DataFitSurrModel::inactive_view(short view, bool recurse_flag)
{
  Model::inactive_view(view);
  if (recurse_flag && !actualModel.is_null())
    actualModel.inactive_view(view, recurse_flag);
}


inline const String& DataFitSurrModel::interface_id() const
{ return approxInterface.interface_id(); }


inline bool DataFitSurrModel::evaluation_cache(bool recurse_flag) const
{
  return (recurse_flag && !actualModel.is_null()) ?
    actualModel.evaluation_cache(recurse_flag) : false;
}


inline bool DataFitSurrModel::restart_file(bool recurse_flag) const
{
  return (recurse_flag && !actualModel.is_null()) ?
    actualModel.restart_file(recurse_flag) : false;
}


inline void DataFitSurrModel::set_evaluation_reference()
{
  approxInterface.set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (!actualModel.is_null())
  //  actualModel.set_evaluation_reference();

  // may want to add this in time
  //surrModelEvalRef = surrModelEvalCntr;
}


inline void DataFitSurrModel::fine_grained_evaluation_counters()
{
  approxInterface.fine_grained_evaluation_counters(numFns);
  if (!actualModel.is_null())
    actualModel.fine_grained_evaluation_counters();
}


inline void DataFitSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  approxInterface.print_evaluation_summary(s, minimal_header, relative_count);
  if (!actualModel.is_null()) {
    if (daceIterator.is_null())
      actualModel.print_evaluation_summary(s, minimal_header, relative_count);
    else // daceIterator resets the eval reference -> don't use a relative count
      actualModel.print_evaluation_summary(s, minimal_header, false);
  }
}


inline void DataFitSurrModel::warm_start_flag(const bool flag)
{
  warmStartFlag = flag;
  if (!actualModel.is_null()) actualModel.warm_start_flag(flag);
}

} // namespace Dakota

#endif
