/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_MODEL_H
#define DAKOTA_MODEL_H

#include "dakota_data_types.hpp"
#include "model_utils.hpp"
#include "MPIManager.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include "DakotaConstraints.hpp"
//#include "DakotaInterface.hpp"
#include "DakotaResponse.hpp"
#include "MultivariateDistribution.hpp"
#include "ScalingOptions.hpp"

namespace Pecos { /* forward declarations */
class SurrogateData;
class ProbabilityTransformation;
class ActiveKey;
}

namespace Dakota {

// define special values for serve_init_mapping()
#define FREE_COMMS 1
#define INIT_COMMS 2
#define SERVE_RUN 3
#define ESTIMATE_MESSAGE_LENGTHS 4

// forward declarations
class ParallelLibrary;
class Approximation;
class SharedApproxData;
class DiscrepancyCorrection;
class EvaluationStore;

extern ParallelLibrary dummy_lib;       // defined in dakota_global_defs.cpp
extern ProblemDescDB   dummy_db;        // defined in dakota_global_defs.cpp


/// Base class for the model class hierarchy.

/** The Model class is the base class for one of the primary
    class hierarchies in DAKOTA.  The model hierarchy contains a set
    of variables, an interface, and a set of responses, and an
    iterator operates on the model to map the variables into responses
    using the interface.  For memory efficiency and enhanced
    polymorphism, the model hierarchy employs the "letter/envelope
    idiom" (see Coplien "Advanced C++", p. 133), for which the base
    class (Model) serves as the envelope and one of the derived
    classes (selected in Model::get_model()) serves as the letter. */

class Model
{

public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Model();
  /// standard constructor
  Model(ProblemDescDB& problem_db);
  /// copy constructor
  Model(const Model& model) = delete;

  /// assignment operator
  Model& operator=(const Model& model) = delete;

  //
  //- Heading: Virtual functions
  //

  virtual ~Model() = default;

  // *** BASE MODEL (BROADLY USED, OPERATES ON BASE DATA, SUPPORTS RECURSION)

  /// return the sub-iterator in nested and surrogate models
  virtual std::shared_ptr<Iterator> subordinate_iterator();
  /// return a single sub-model defined from subModel in nested and recast
  /// models and truth_model() in surrogate models; used for a directed
  /// dive through model recursions that may bypass some components.
  virtual std::shared_ptr<Model> subordinate_model();

  /// portion of subordinate_models() specific to derived model classes
  virtual void derived_subordinate_models(ModelList& ml, bool recurse_flag);
  /// resize vars/resp if needed from the bottom up
  virtual void resize_from_subordinate_model(size_t depth = SZ_MAX);
  /// propagate vars/labels/bounds/targets from the bottom up
  virtual void update_from_subordinate_model(size_t depth = SZ_MAX);
  /// return the interface employed by the derived model class, if present:
  /// SimulationModel::userDefinedInterface, DataFitSurrModel::approxInterface,
  /// or NestedModel::optionalInterface
  virtual std::shared_ptr<Interface> derived_interface();
  virtual void derived_interface(std::shared_ptr<Interface> );

  /// set the relative weightings for multiple objective functions or least
  /// squares terms
  virtual void primary_response_fn_weights(const RealVector& wts, 
					   bool recurse_flag = true);

  /// Perform any global updates prior to individual evaluate() calls;
  /// returns true if the variables size has changed
  virtual bool initialize_mapping(ParLevLIter pl_iter);
  /// restore state in preparation for next initialization;
  /// returns true if the variables size has changed
  virtual bool finalize_mapping();

  /// called from IteratorScheduler::run_iterator() for iteratorComm rank 0 to
  /// terminate serve_init_mapping() on other iteratorComm processors
  virtual void stop_init_mapping(ParLevLIter pl_iter);
  /// called from IteratorScheduler::run_iterator() for iteratorComm rank != 0
  /// to balance resize() calls on iteratorComm rank 0
  virtual int serve_init_mapping(ParLevLIter pl_iter);

  /// called from IteratorScheduler::run_iterator() for iteratorComm rank 0 to
  /// terminate serve_finalize_mapping() on other iteratorComm processors
  virtual void stop_finalize_mapping(ParLevLIter pl_iter);
  /// called from IteratorScheduler::run_iterator() for iteratorComm rank != 0
  /// to balance resize() calls on iteratorComm rank 0
  virtual int serve_finalize_mapping(ParLevLIter pl_iter);

  /// retrieve error estimates corresponding to the Model's response
  /// (could be surrogate error for SurrogateModels, statistical MSE for
  /// NestedModels, or adjoint error estimates for SimulationModels).
  /// Errors returned correspond to most recent evaluate().
  /// TO DO: why not yet bound to SurrogateModel::approximation_variances()?
  virtual const RealVector& error_estimates();

  /// update componentParallelMode for supporting parallelism in model
  /// sub-components
  virtual void component_parallel_mode(short mode);

  /// estimate the minimum and maximum partition sizes that can be
  /// utilized by this Model
  virtual IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// return the index for the metaiterator-iterator parallelism level within
  /// ParallelConfiguration::miPLIters that is active for use in a particular
  /// Model at runtime
  virtual size_t mi_parallel_level_index() const;

  /// migrate an unmatched response record from active response map (computed
  /// by synchronize() or synhronize_nowait()) to cached response map
  virtual void cache_unmatched_response(int raw_id);
  /// migrate remaining response records from responseMap to cachedResponseMap
  virtual void cache_unmatched_responses();

  /// return derived model synchronization setting
  virtual short local_eval_synchronization();
  /// return derived model asynchronous evaluation concurrency
  virtual int local_eval_concurrency();
  /// return derived model synchronization setting
  virtual void serialize_threshold(size_t thresh);

  /// Service job requests received from the scheduler.  Completes when
  /// a termination message is received from stop_servers().
  virtual void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);
  /// Executed by the scheduler to terminate all server operations for a
  /// particular model when iteration on the model is complete.
  virtual void stop_servers();

  /// Return a flag indicating the combination of multiprocessor evaluations
  /// and a dedicated scheduler processor for iterator-model scheduling.  Used
  /// in synchronous evaluate functions to prevent the error of trying to run
  /// a multiprocessor job on the dedicated scheduler.
  virtual bool derived_scheduler_overload() const;

  /// create 2D graphics plots for automatic logging of vars/response data
  virtual void create_2d_plots();
  /// create a tabular output stream for automatic logging of vars/response data
  virtual void create_tabular_datastream();

  /// Update tabular/graphics data with latest variables/response data
  virtual void derived_auto_graphics(const Variables& vars,
				     const Response& resp);

  /// update the Model's active view based on a higher level context
  virtual void active_view(short view, bool recurse_flag = true);
  /// update the Model's inactive view based on a higher level context
  virtual void inactive_view(short view, bool recurse_flag = true);

  /// return the interface identifier
  virtual const String& interface_id() const;
  /// Return the value of the evaluation id counter for the Model
  virtual int derived_evaluation_id() const;

  /// Indicates the usage of an evaluation cache by the Model
  virtual bool evaluation_cache(bool recurse_flag = true) const;
  /// Indicates the usage of a restart file by the Model
  virtual bool restart_file(bool recurse_flag = true) const;

  /// Set the reference points for the evaluation counters within the Model
  virtual void set_evaluation_reference();
  /// Request fine-grained evaluation reporting within the Model
  virtual void fine_grained_evaluation_counters();
  /// Print an evaluation summary for the Model
  virtual void print_evaluation_summary(std::ostream& s,
					bool minimal_header = false,
					bool relative_count = true) const;

  /// set the hierarchical eval ID tag prefix
  virtual void eval_tag_prefix(const String& eval_id_str);

  /// search the eval database (during derivative estimation); derived
  /// may need to reimplement due to problem transformations
  /// (RecastModel); return true if found in DB
  virtual bool db_lookup(const Variables& search_vars, 
			 const ActiveSet& search_set, Response& found_resp);

  /// set the warm start flag (warmStartFlag)
  virtual void warm_start_flag(const bool flag);

  /// Declare a model's sources to the evaluationsDB
  virtual void declare_sources();

  /// set primaryA{C,DI,DS,DR}VarMapIndices, secondaryA{C,DI,DS,DR}VarMapTargets
  /// (coming from a higher-level NestedModel context to inform derivative est.)
  virtual void nested_variable_mappings(const SizetArray& c_index1,
					const SizetArray& di_index1,
					const SizetArray& ds_index1,
					const SizetArray& dr_index1,
					const ShortArray& c_target2,
					const ShortArray& di_target2,
					const ShortArray& ds_target2,
					const ShortArray& dr_target2);
  /// return primaryACVarMapIndices
  virtual const SizetArray& nested_acv1_indices() const;
  /// return secondaryACVarMapTargets
  virtual const ShortArray& nested_acv2_targets() const;

  /// calculate and return derivative composition of final results
  /// w.r.t. distribution parameters (none, all, or mixed)
  virtual short query_distribution_parameter_derivatives() const;
  /// activate derivative setting w.r.t. distribution parameters
  virtual void activate_distribution_parameter_derivatives();
  /// deactivate derivative setting w.r.t. distribution parameters
  virtual void deactivate_distribution_parameter_derivatives();

  // *** TO DO: these transformation fns are recursive but minimally so -- could
  //     we retire direct uses of these, relying more on transform recursions?
  //     (e.g., older use cases in NonDLocalReliability)

  /// return probability transformation employed by the Model (forwarded along
  /// to ProbabilityTransformModel recasting)
  virtual Pecos::ProbabilityTransformation& probability_transformation();

  /// transform u-space variable values to x-space
  virtual void trans_U_to_X(const RealVector& u_c_vars, RealVector& x_c_vars);
  /// transform x-space variable values to u-space
  virtual void trans_X_to_U(const RealVector& x_c_vars, RealVector& u_c_vars);

  /// transform x-space gradient vector to u-space
  virtual void trans_grad_X_to_U(const RealVector& fn_grad_x,
				 RealVector& fn_grad_u,
				 const RealVector& x_vars);
  /// transform u-space gradient vector to x-space
  virtual void trans_grad_U_to_X(const RealVector& fn_grad_u,
				 RealVector& fn_grad_x,
				 const RealVector& x_vars);
  /// transform x-space gradient vector to gradient with respect to inserted
  /// distribution parameters
  virtual void trans_grad_X_to_S(const RealVector& fn_grad_x,
				 RealVector& fn_grad_s,
				 const RealVector& x_vars);
  /// transform x-space Hessian matrix to u-space
  virtual void trans_hess_X_to_U(const RealSymMatrix& fn_hess_x,
				 RealSymMatrix& fn_hess_u,
				 const RealVector& x_vars,
				 const RealVector& fn_grad_x);


  // *** SURROGATE MODELS (BOTH DATA FIT AND ENSEMBLE)
  // *** Note: RecastModels will implement forwards (TO DO: verify there is no interaction with recasting)

  /// return number of unique response functions (managing any aggregations)
  virtual size_t qoi() const;

  /// return the i-th approximation sub-model in surrogate models
  virtual std::shared_ptr<Model> surrogate_model(size_t i = _NPOS);
  /// return the i-th approximation sub-model in surrogate models
  virtual std::shared_ptr<const Model> surrogate_model(size_t i = _NPOS) const;
  /// return the truth sub-model in surrogate models
  virtual std::shared_ptr<Model> truth_model();
  /// return the truth sub-model in surrogate models
  virtual std::shared_ptr<const Model> truth_model() const;

  /// return the model form of the i-th active surrogate model
  virtual unsigned short active_surrogate_model_form(size_t i) const;
  /// return the model form of the active truth model
  virtual unsigned short active_truth_model_form() const;

  /// return the i-th active approximation sub-model in surrogate models
  virtual std::shared_ptr<Model> active_surrogate_model(size_t i = _NPOS);
  /// return the i-th active approximation sub-model in surrogate models
  virtual std::shared_ptr<const Model> active_surrogate_model(size_t i = _NPOS) const;
  /// return the active truth sub-model in surrogate models
  virtual std::shared_ptr<Model> active_truth_model();
  /// return the active truth sub-model in surrogate models
  virtual std::shared_ptr<const Model> active_truth_model() const;

  /// set the active model key within surrogate data, grid driver,
  /// and approximation classes that support the management of multiple
  /// approximation states within surrogate models
  virtual void active_model_key(const Pecos::ActiveKey& key);
  /// return the active model key (used by surrogate data, grid driver,
  /// and approximation classes to support the management of multiple
  /// approximation states within surrogate models)
  virtual const Pecos::ActiveKey& active_model_key() const;
  /// reset by removing all model keys within surrogate data, grid driver,
  /// and approximation classes that support the management of multiple
  /// approximation states within surrogate models
  virtual void clear_model_keys();

  /// return true if there is an active truth model indicated by truthModelKey
  virtual bool active_truth_key() const;
  /// return the number of active surrogate models indicated by surrModelKeys
  virtual size_t active_surrogate_keys() const;

  /// build a new SurrogateModel approximation
  virtual void build_approximation();

  /// determine whether a surrogate model rebuild should be forced
  /// based on changes in the inactive data
  virtual bool force_rebuild();

    /// set the (currently active) surrogate function index set
  virtual void surrogate_function_indices(const SizetSet& surr_fn_indices);

  /// set response computation mode used in SurrogateModels for
  /// forming currentResponse
  virtual void surrogate_response_mode(short mode);
  /// return response computation mode used in SurrogateModels for
  /// forming currentResponse
  virtual short surrogate_response_mode() const;

  /// return the DiscrepancyCorrection object used by SurrogateModels
  virtual DiscrepancyCorrection& discrepancy_correction();
  /// set the correction type from the DiscrepancyCorrection object
  /// used by SurrogateModels
  virtual void correction_type(short corr_type);
  /// return the correction type from the DiscrepancyCorrection object
  /// used by SurrogateModels
  virtual short correction_type() const;
  /// return the correction order from the DiscrepancyCorrection object
  /// used by SurrogateModels
  virtual short correction_order() const;
  /// return correctionMode
  virtual unsigned short correction_mode() const;
  /// set correctionMode
  virtual void correction_mode(unsigned short corr_mode);


  // *** DATA FIT SURROGATE MODELS

  /// build a new SurrogateModel approximation using/enforcing
  /// anchor response at vars; rebuild if needed
  virtual bool build_approximation(const Variables& vars,
				   const IntResponsePair& response_pr);

  /// incremental rebuild of an existing SurrogateModel approximation
  virtual void rebuild_approximation();
  /// incremental rebuild of an existing SurrogateModel approximation
  virtual void rebuild_approximation(const IntResponsePair& response_pr);
  /// incremental rebuild of an existing SurrogateModel approximation
  virtual void rebuild_approximation(const IntResponseMap& resp_map);

  /// replace the approximation data within an existing surrogate
  /// based on data updates propagated elsewhere
  virtual void update_approximation(bool rebuild_flag);
  /// replace the anchor point data within an existing surrogate
  virtual void update_approximation(const Variables& vars, 
				    const IntResponsePair& response_pr,
				    bool rebuild_flag);
  /// replace the data points within an existing surrogate
  virtual void update_approximation(const VariablesArray& vars_array,
				    const IntResponseMap& resp_map,
				    bool rebuild_flag);
  /// replace the data points within an existing surrogate
  virtual void update_approximation(const RealMatrix& samples,
				    const IntResponseMap& resp_map,
				    bool rebuild_flag);

  /// append to the existing approximation data within a surrogate
  /// based on data updates propagated elsewhere
  virtual void append_approximation(bool rebuild_flag);
  /// append a single point to an existing surrogate's data
  virtual void append_approximation(const Variables& vars, 
				    const IntResponsePair& response_pr,
				    bool rebuild_flag);
  /// append multiple points to an existing surrogate's data
  virtual void append_approximation(const RealMatrix& samples,
				    const IntResponseMap& resp_map,
				    bool rebuild_flag);
  /// append multiple points to an existing surrogate's data
  virtual void append_approximation(const VariablesArray& vars_array,
				    const IntResponseMap& resp_map,
				    bool rebuild_flag);
  /// append multiple points to an existing surrogate's data
  virtual void append_approximation(const IntVariablesMap& vars_map,
				    const IntResponseMap&  resp_map,
				    bool rebuild_flag);

  /// replace the response for a single point (based on eval id from
  /// response_pr) within an existing surrogate's data
  virtual void replace_approximation(const IntResponsePair& response_pr,
				     bool rebuild_flag);
  /// replace the responses for a set of points (based on eval ids from
  /// resp_map) within an existing surrogate's data
  virtual void replace_approximation(const IntResponseMap& resp_map,
				     bool rebuild_flag);
  /// assigns a flag to track evaluation ids within surrogate data,
  /// enabling id-based lookups for data replacement
  virtual void track_evaluation_ids(bool track);

  /// remove the previous data set addition to a surrogate (e.g., due
  /// to a previous append_approximation() call); flag manages storing
  /// of surrogate data for use in a subsequent push_approximation()
  virtual void pop_approximation(bool save_surr_data,
				 bool rebuild_flag = false);

  /// push a previous approximation data state; reverse of pop_approximation
  virtual void push_approximation();
  /// query for whether a trial increment is restorable within a surrogate
  virtual bool push_available();

  /// finalize an approximation by applying all previous trial increments
  virtual void finalize_approximation();

  /// combine the current approximation with previously stored data sets
  virtual void combine_approximation();
  /// promote the combined approximation into the active approximation
  virtual void combined_to_active(bool clear_combined = true);
  /// clear inactive approximations (finalization + combination completed)
  virtual void clear_inactive();

  /// query the approximation for available advancement in resolution controls
  /// (order, rank, etc.); an input to adaptive refinement strategies
  virtual bool advancement_available();
  /// query the approximation for updates in formulation, requiring a rebuild
  /// even if no updates to the build data
  virtual bool formulation_updated() const;
  /// assign the status of approximation formulation updates
  virtual void formulation_updated(bool update);

  /// execute the DACE iterator (prior to building/appending the approximation)
  virtual void run_dace();

  // retrieve the variables used to build a surrogate model
  //virtual const VariablesArray build_variables() const;
  // retrieve the responses used to build a surrogate model
  //virtual const ResponseArray build_responses() const;

  /// retrieve the shared approximation data within the ApproximationInterface
  /// of a DataFitSurrModel
  virtual SharedApproxData& shared_approximation();
  /// retrieve the set of Approximations within the ApproximationInterface
  /// of a DataFitSurrModel
  virtual std::vector<Approximation>& approximations();
  /// retrieve a SurrogateData instance from a particular Approximation
  /// instance within the ApproximationInterface of a DataFitSurrModel
  virtual const Pecos::SurrogateData& approximation_data(size_t fn_index);

  /// retrieve the approximation coefficients from each Approximation
  /// within a DataFitSurrModel
  virtual const RealVectorArray&
    approximation_coefficients(bool normalized = false);
  /// set the approximation coefficients for each Approximation within
  /// a DataFitSurrModel
  virtual void approximation_coefficients(const RealVectorArray& approx_coeffs,
					  bool normalized = false);

  /// retrieve the prediction variances from each Approximation within
  /// a DataFitSurrModel
  virtual const RealVector& approximation_variances(const Variables& vars);

  /// set discrepancy emulation mode used in SurrogateModels for
  /// approximating response differences
  virtual void discrepancy_emulation_mode(short mode);
  // return discrepancy emulation mode used in SurrogateModels for
  // approximating response differences
  //virtual short discrepancy_emulation_mode() const;

  // link together more than one SurrogateData instance (DataFitSurrModel)
  //virtual void link_multilevel_approximation_data();


  // *** ENSEMBLE SURROGATE MODELS

  /// identify if 1D hierarchy can be defined across model forms
  virtual bool multifidelity() const;
  /// identify if 1D hierarchy can be defined across resolution levels
  virtual bool multilevel() const;
  /// identify if 2D hierarchy can be defined across both model forms
  /// and resolution levels
  virtual bool multilevel_multifidelity() const;

  /// return precedence for ensemble definition: model forms, resolution
  /// levels, or both
  virtual short ensemble_precedence() const;
  /// assign precedence for ensemble definition (model forms or
  /// resolution levels or both) as determined from algorithm context
  virtual void ensemble_precedence(short mlmf_prec,
				   bool update_default = false);

  /// apply a DiscrepancyCorrection to correct an approximation within
  /// an EnsembleSurrModel
  virtual void single_apply(const Variables& vars, Response& resp,
			    const Pecos::ActiveKey& paired_key);
  /// apply a sequence of DiscrepancyCorrections to recursively correct an 
  /// approximation within an EnsembleSurrModel
  virtual void recursive_apply(const Variables& vars, Response& resp);


  // *** SIMULATION MODELS

  /// number of discrete levels within solution control (SimulationModel)
  virtual size_t solution_levels() const;
  /// activate a particular level within the solution level control
  /// (SimulationModel)
  virtual void solution_level_cost_index(size_t index);
  /// return currently active level within the solution level control
  /// (SimulationModel)
  virtual size_t solution_level_cost_index() const;
  /// return ordered cost estimates across solution levels (SimulationModel)
  virtual RealVector solution_level_costs() const;
  /// return currently active cost estimate from solution level
  /// control (SimulationModel)
  virtual Real solution_level_cost() const;

  /// return type of solution control variable
  virtual short solution_control_variable_type() const;
  /// return index of solution control variable within all variables
  virtual size_t solution_control_variable_index() const;
  /// return index of solution control variable within all discrete variables
  virtual size_t solution_control_discrete_variable_index() const;

  /// return the active (integer) value of the solution control
  virtual int    solution_level_int_value() const;
  /// return the active (string) value of the solution control
  virtual String solution_level_string_value() const;
  /// return the active (real) value of the solution control
  virtual Real   solution_level_real_value() const;

  /// return index of online cost estimates within metadata
  virtual size_t cost_metadata_index() const;


  // *** RECAST MODELS

  /// return true if a potential resize is still pending, such that
  /// sizing-based initialization should be deferred
  virtual bool resize_pending() const;


  // *** PROBABILITY TRANSFORM MODELS

  // see weakly recursive list for base Model above


  // *** NESTED MODELS

  // see recursive list for base Model above


  //
  //- Heading: Member functions
  //

  // *** BASE MODEL (BROADLY USED, OPERATES ON BASE DATA)

  /// return the sub-models in nested and surrogate models
  ModelList& subordinate_models(bool recurse_flag = true);

  // TO DO: synchronous evaluate() should return int codes for
  // algorithm-specific mitigation of captured failures

  /// Compute the Response at currentVariables (default ActiveSet).
  void evaluate();
  /// Compute the Response at currentVariables (specified ActiveSet).
  void evaluate(const ActiveSet& set);

  /// Spawn an asynchronous job (or jobs) that computes the value of the 
  /// Response at currentVariables (default ActiveSet).
  void evaluate_nowait();
  /// Spawn an asynchronous job (or jobs) that computes the value of the 
  /// Response at currentVariables (specified ActiveSet).
  void evaluate_nowait(const ActiveSet& set);

  // TO DO: for evaluate_nowait(), add access fns for ShortArray/List
  // return codes for algorithm-specific mitigation of captured failures.

  /// Execute a blocking scheduling algorithm to collect the
  /// complete set of results from a group of asynchronous evaluations.
  const IntResponseMap& synchronize();
  /// Execute a nonblocking scheduling algorithm to collect all
  /// available results from a group of asynchronous evaluations.
  const IntResponseMap& synchronize_nowait();

  /// return Model's (top-level) evaluation counter, not to be confused
  /// with derived counter returned by derived_evaluation_id()
  int evaluation_id() const;

  // return mappingInitialized
  bool mapping_initialized() const;

  /// allocate communicator partitions for a model and store
  /// configuration in modelPCIterMap
  void init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag = true);
  /// for cases where init_communicators() will not be called,
  /// modify some default settings to behave properly in serial.
  void init_serial();
  /// set active parallel configuration for the model (set modelPCIter
  /// from modelPCIterMap)
  void set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			 bool recurse_flag = true);
  /// deallocate communicator partitions for a model
  void free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag = true);

  /// retrieve the MPI communicator on which this model is configured to 
  /// conduct function evaluation analyses (provided for library clients)
  MPI_Comm analysis_comm() const;

  /// called from IteratorScheduler::init_iterator() for iteratorComm rank 0 to
  /// terminate serve_init_communicators() on other iteratorComm processors
  void stop_init_communicators(ParLevLIter pl_iter);
  /// called from IteratorScheduler::init_iterator() for iteratorComm rank != 0
  /// to balance init_communicators() calls on iteratorComm rank 0
  int serve_init_communicators(ParLevLIter pl_iter);

  /// estimate messageLengths for a model
  void estimate_message_lengths();

  /// initialize modelList and recastFlags for data import/export
  bool manage_data_recastings();
  /// return true if recastFlags is defined
  bool recastings() const;

  /// employ the model recursion to transform from bottom level
  /// user-space variables to top level iterator-space variables
  void user_space_to_iterator_space(Variables& vars);
  /// employ the model recursion to transform from bottom level
  /// user-space data to top level iterator-space data
  void user_space_to_iterator_space(const Variables& user_vars,
				    const Response&  user_resp,
				    Variables& iter_vars, Response& iter_resp);
  /// employ the model recursion to transform from top level
  /// iterator-space variables to bottom level user-space variables
  void iterator_space_to_user_space(Variables& vars);
  /// employ the model recursion to transform from top level
  /// iterator-space data to bottom level user-space data
  void iterator_space_to_user_space(const Variables& iter_vars,
				    const Response&  iter_resp,
				    Variables& user_vars, Response& user_resp);
 
  /// return mvDist
  Pecos::MultivariateDistribution& multivariate_distribution();
  /// return mvDist
  const Pecos::MultivariateDistribution& multivariate_distribution() const;
  // set mvDist
  //void multivariate_distribution(const Pecos::MultivariateDistribution& dist);

  /// return the current variables (currentVariables) as const
  /// reference (preferred)
  const Variables& current_variables() const;
  /// return the current variables (currentVariables) in mutable form
  /// (special cases)
  Variables& current_variables();
  /// return the user-defined constraints (userDefinedConstraints) as const
  /// reference (preferred)
  const Constraints& user_defined_constraints() const;
  /// return the user-defined constraints (userDefinedConstraints) in
  /// mutable form (special cases)
  Constraints& user_defined_constraints();
  /// return the current response (currentResponse)
  const Response& current_response() const;
  /// return the current response (currentResponse)
  Response& current_response();
  /// return the problem description database (probDescDB)
  ProblemDescDB& problem_description_db() const;
  /// return the parallel library (parallelLib)
  ParallelLibrary& parallel_library() const;
  /// return the model type (modelType)
  const String& model_type() const;
  /// return the surrogate type (surrogateType)
  const String& surrogate_type() const;
  /// return the model identifier (modelId)
  const String& model_id() const;

  /// return number of primary functions (total less nonlinear constraints)
  size_t num_primary_fns() const;
  /// return number of secondary functions (number of nonlinear constraints)
  size_t num_secondary_fns() const;

  /// return the gradient evaluation type (gradientType)
  const String& gradient_type() const;
  /// return the numerical gradient evaluation method source (methodSource)
  const String& method_source() const;
  /// return the numerical gradient evaluation interval type (intervalType)
  const String& interval_type() const;
  /// option for ignoring bounds when numerically estimating derivatives
  bool ignore_bounds() const;
  /// option for using old 2nd-order scheme when computing finite-diff Hessian
  bool central_hess() const;
  /// return the finite difference gradient step size (fdGradStepSize)
  const RealVector& fd_gradient_step_size() const;
  /// return the finite difference gradient step type (fdGradStepType)
  const String& fd_gradient_step_type() const;
  /// return the mixed gradient analytic IDs (gradIdAnalytic)
  const IntSet& gradient_id_analytic() const;
  /// return the mixed gradient numerical IDs (gradIdNumerical)
  const IntSet& gradient_id_numerical() const;

  /// return the Hessian evaluation type (hessianType)
  const String& hessian_type() const;
  /// return the Hessian evaluation type (quasiHessType)
  const String& quasi_hessian_type() const;
  /// return gradient-based finite difference Hessian step size
  /// (fdHessByGradStepSize)
  const RealVector& fd_hessian_by_grad_step_size() const;
  /// return function-based finite difference Hessian step size
  /// (fdHessByFnStepSize)
  const RealVector& fd_hessian_by_fn_step_size() const;
  /// return the finite difference Hessian step type (fdHessStepType)
  const String& fd_hessian_step_type() const;
  /// return the mixed Hessian analytic IDs (hessIdAnalytic)
  const IntSet& hessian_id_analytic() const;
  /// return the mixed Hessian analytic IDs (hessIdNumerical)
  const IntSet& hessian_id_numerical() const;
  /// return the mixed Hessian analytic IDs (hessIdQuasi)
  const IntSet& hessian_id_quasi() const;

  /// set the optimization sense for multiple objective functions
  void primary_response_fn_sense(const BoolDeque& sense);
  /// get the optimization sense for multiple objective functions
  const BoolDeque& primary_response_fn_sense() const;
  /// get the relative weightings for multiple objective functions or least
  /// squares terms
  const RealVector& primary_response_fn_weights() const;
  /// user-provided scaling options
  const ScalingOptions& scaling_options() const;

  /// get the primary response function type (generic, objective, calibration)
  short primary_fn_type() const;
  /// set the primary response function type, e.g., when recasting
  void primary_fn_type(short type);

  /// indicates potential usage of estimate_derivatives() based on
  /// gradientType/hessianType
  bool derivative_estimation();
  /// set whether this model should perform or pass on derivative estimation
  void supports_derivative_estimation(bool sed_flag);

  /// set initCommsBcastFlag
  void init_comms_bcast_flag(bool icb_flag);

  /// return the evaluation capacity for use in iterator logic
  int evaluation_capacity() const;

  /// return the gradient concurrency for use in parallel configuration logic
  int derivative_concurrency() const;

  /// return the asynchronous evaluation flag (asynchEvalFlag)
  bool asynch_flag() const;
  /// set the asynchronous evaluation flag (asynchEvalFlag)
  void asynch_flag(const bool flag);

  /// return the outputLevel
  short output_level() const;
  /// set the outputLevel
  void output_level(const short level);

  /// return the array of MPI packed message buffer lengths (messageLengths)
  const IntArray& message_lengths() const;

  /// set modelPCIter
  void parallel_configuration_iterator(ParConfigLIter pc_iter);
  /// return modelPCIter
  ParConfigLIter parallel_configuration_iterator() const;

  /// set modelAutoGraphicsFlag to activate posting of graphics data within
  /// evaluate/synchronize functions (automatic graphics posting in
  /// the model as opposed to graphics posting at the strategy level).
  void auto_graphics(const bool flag);

  /// get modelAutoGraphicsFlag to activate posting of graphics data within
  /// evaluate/synchronize functions (automatic graphics posting in
  /// the model as opposed to graphics posting at the strategy level).
  bool auto_graphics() const;

  /// set the specified configuration to the Model's inactive vars, converting
  /// from real to integer or through index to string value as needed
  static void active_variables(const RealVector& config_vars, Model& model);
  /// set the specified configuration to the Model's inactive vars, converting
  /// from real to integer or through index to string value as needed
  static void inactive_variables(const RealVector& config_vars, Model& model);

  static void inactive_variables(const RealVector& config_vars, Model& model,
				 Variables& updated_vars);

  /// Bulk synchronously evaluate the model for each column (of active
  /// variables) in the samples matrix and return as columns of the
  /// response matrix
  static void evaluate(const RealMatrix& samples_matrix,
		       Model& model, RealMatrix& resp_matrix);

  /// Bulk synchronously evaluate the model for each entry (of active
  /// variables) in the samples vector and return as columns of the
  /// response matrix
  static void evaluate(const VariablesArray& sample_vars,
		       Model& model, RealMatrix& resp_matrix);

  /// Return the model ID of the "innermost" model. 
  /// For all derived Models except RecastModels, return modelId.
  /// The RecastModel override returns the root_model_id() of the subModel.
  virtual String root_model_id();

  virtual ActiveSet default_active_set();

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializing the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  // Model(ProblemDescDB& problem_db);

  /// constructor initializing base class for derived model class instances
  /// constructed on the fly
  Model(const ShortShortPair& vars_view,
	const SharedVariablesData& svd, bool share_svd,
	const SharedResponseData& srd, bool share_srd, const ActiveSet& set,
	short output_level, ProblemDescDB& problem_db = dummy_db,
	ParallelLibrary& parallel_lib = dummy_lib);

  /// constructor initializing base class for recast model instances
  Model(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib);

  //
  //- Heading: Virtual functions
  //

  // *** BASE MODEL (BROADLY USED, OPERATES ON BASE DATA, SUPPORTS RECURSION)

  /// portion of evaluate() specific to derived model classes
  virtual void derived_evaluate(const ActiveSet& set);
  /// portion of evaluate_nowait() specific to derived model classes
  virtual void derived_evaluate_nowait(const ActiveSet& set);

  /// portion of synchronize() specific to derived model classes
  virtual const IntResponseMap& derived_synchronize();
  /// portion of synchronize_nowait() specific to derived model classes
  virtual const IntResponseMap& derived_synchronize_nowait();

  /// portion of init_communicators() specific to derived model classes
  virtual void derived_init_communicators(ParLevLIter pl_iter,
					  int max_eval_concurrency,
					  bool recurse_flag = true);
  /// portion of init_serial() specific to derived model classes
  virtual void derived_init_serial();
  /// portion of set_communicators() specific to derived model classes
  virtual void derived_set_communicators(ParLevLIter pl_iter,
					 int max_eval_concurrency,
					 bool recurse_flag = true);
  /// portion of free_communicators() specific to derived model classes
  virtual void derived_free_communicators(ParLevLIter pl_iter,
					  int max_eval_concurrency,
					  bool recurse_flag = true);

  //
  //- Heading: Member functions
  //

  // *** BASE MODEL (BROADLY USED, OPERATES ON BASE DATA)

  /// update incoming (sub-)model with active values from currentVariables
  void update_model_active_variables(Model& model);

  /// return responseMap
  IntResponseMap& response_map();

  /// return the next available model ID for no-ID user methods
  static String user_auto_id();
  /// return the next available model ID for on-the-fly methods
  static String no_spec_id();

  /// initialize distribution types from problemDescDB
  void initialize_distribution(
    Pecos::MultivariateDistribution& mv_dist, bool active_only = false);
  /// initialize distribution types from problemDescDB
  void initialize_active_types(Pecos::MultivariateDistribution& mv_dist);
  /// initialize distribution parameters from problemDescDB
  void initialize_distribution_parameters(
    Pecos::MultivariateDistribution& mv_dist, bool active_only = false);

  /// default logic for defining asynchEvalFlag and evaluationCapacity
  /// based on ie_pl settings
  void set_ie_asynchronous_mode(int max_eval_concurrency);

  /// assign all of the longest possible string values into vars
  void assign_max_strings(const Pecos::MultivariateDistribution& mv_dist,
			  Variables& vars);
  /// return iterator for longest string value found in string set
  SSCIter  max_string(const StringSet& ss);
  /// return iterator for longest string value found in string map
  SRMCIter max_string(const StringRealMap& srm);

  /// Initialize data needed for computing finite differences
  /// (active/inactive, center point, and bounds)
  SizetMultiArrayConstView
  initialize_x0_bounds(const SizetArray& original_dvv, bool& active_derivs, 
                       bool& inactive_derivs, RealVector& x0, RealVector& fd_lb,
                       RealVector& fd_ub) const;

  /// Compute the forward step for a finite difference gradient;
  /// updates shortStep
  Real forward_grad_step(size_t num_deriv_vars, size_t xj_index,
                         Real x0_j, Real lb_j, Real ub_j);

  /// Return the interface flag for the EvaluationsDB state
  EvaluationsDBState evaluations_db_state(const Interface &interface);
  /// Return the model flag for the EvaluationsDB state
  EvaluationsDBState evaluations_db_state(const Model &model);

  /// Store the response portion of an interface evaluation.
  /// Called from rekey_response_map()
  void asynch_eval_store(const Interface &interface, const int &id,
			 const Response &response);
  /// Exists to support storage of interface evaluations.
  /// No-op so that rekey_response_map<Model> can be generated.
  void asynch_eval_store(const Model &model, const int &id,
			 const Response &response);

  /// rekey returned jobs matched in array of id_maps into array of
  /// resp_maps_rekey; unmatched jobs can be cached within the meta_object
  template <typename MetaType>
  void rekey_response_map(MetaType& meta_object, IntIntMapArray& id_maps,
			  IntResponseMapArray& resp_maps_rekey, bool deep_copy);
  /// rekey returned jobs matched in id_map into resp_map_rekey;
  /// unmatched jobs can be cached within the meta_object
  template <typename MetaType>
  void rekey_response_map(MetaType& meta_object, IntIntMap& id_map,
			  IntResponseMap& resp_map_rekey, bool deep_copy);

  /// synchronize via meta_object and rekey returned jobs matched in array of
  /// id_maps into array of resp_maps_rekey; unmatched jobs are cached within
  /// the meta_object
  template <typename MetaType>
  void rekey_synch(MetaType& meta_object, bool block, IntIntMapArray& id_maps,
		   IntResponseMapArray& resp_maps_rekey,
		   bool deep_copy = false);
  /// synchronize via meta_object and rekey returned jobs matched in id_map
  /// into resp_map_rekey; unmatched jobs are cached within the meta_object
  template <typename MetaType>
  void rekey_synch(MetaType& meta_object, bool block, IntIntMap& id_map,
		   IntResponseMap& resp_map_rekey, bool deep_copy = false);

  //
  //- Heading: Data
  //

  // The 3 parts of the mapping performed by a model. Note that the size_t's
  // describing the sizes of data within these objects must be declared in the
  // order that ModelRep initialization should occur (see Effective C++, p. 41).

  /// the set of current variables used by the model for performing
  /// function evaluations
  Variables currentVariables;
  /// the number of active continuous variables used in computing most response
  /// derivatives (i.e., in places such as quasi-Hessians and response
  /// corrections where only the active continuous variables are supported)
  size_t numDerivVars;

  // presence of a Interface now varies by derived model class
  //Interface userDefinedInterface;

  /// the set of current responses that holds the results of model
  /// function evaluations
  Response currentResponse;
  /// the number of functions in currentResponse
  size_t numFns;

  /// Explicit constraints on variables are maintained in the Constraints
  /// class hierarchy.  Currently, this includes linear constraints and bounds,
  /// but could be extended in the future to include other explicit constraints
  /// which (1) have their form specified by the user, and (2) are not
  /// catalogued in Response since their form and coefficients are published to
  /// an iterator at startup.
  Constraints userDefinedConstraints;

  /// model identifier string from the input file
  String modelId;
  /// type of model: simulation, nested, or surrogate
  String modelType;
  /// type of surrogate model: local_*, multipoint_*, global_*, or hierarchical
  String surrogateType;

  /// type of gradient data: analytic, numerical, mixed, or none
  String gradientType;
  /// source of numerical gradient routine: dakota or vendor
  String methodSource;
  /// type of numerical gradient interval: central or forward
  String intervalType;
  /// type of Hessian data: analytic, numerical, quasi, mixed, or none
  String hessianType;

  /// relative finite difference step size for numerical gradients
  /** A scalar value (instead of the vector fd_gradient_step_size spec) is
      used within the iterator hierarchy since this attribute is only used
      to publish a step size to vendor numerical gradient algorithms. */
  RealVector fdGradStepSize;
  /// type of finite difference step to use for numerical gradient:
  /// relative - step length is relative to x
  /// absolute - step length is what is specified
  /// bounds   - step length is relative to range of x
  String fdGradStepType;
  /// relative finite difference step size for numerical Hessians estimated 
  /// using first-order differences of gradients
  /** For vendor numerical Hessian algorithms, a scalar value is used. */
  RealVector fdHessByGradStepSize;
  /// relative finite difference step size for numerical Hessians estimated 
  /// using second-order differences of function values
  /** For vendor numerical Hessian algorithms, a scalar value is used. */
  RealVector fdHessByFnStepSize;
  /// type of finite difference step to use for numerical Hessian:
  /// relative - step length is relative to x
  /// absolute - step length is what is specified
  /// bounds   - step length is relative to range of x
  String fdHessStepType;

  /// option to ignore bounds when computing finite diffs
  bool ignoreBounds;
  /// option to use old 2nd-order finite diffs for Hessians
  bool centralHess;
  /// if in warm-start mode, don't reset accumulated data (e.g., quasiHessians)
  bool warmStartFlag;
  /// whether model should perform or forward derivative estimation
  bool supportsEstimDerivs;
  /// quasi-Hessian type: bfgs, damped_bfgs, sr1
  String quasiHessType;

  IntSet gradIdAnalytic;  ///< analytic id's for mixed gradients
  IntSet gradIdNumerical; ///< numerical id's for mixed gradients
  IntSet hessIdAnalytic;  ///< analytic id's for mixed Hessians
  IntSet hessIdNumerical; ///< numerical id's for mixed Hessians
  IntSet hessIdQuasi;     ///< quasi id's for mixed Hessians

  /// Whether to write model evals to the evaluations DB
  EvaluationsDBState modelEvaluationsDBState;
  /// Whether to write interface evals to the evaluations DB
  EvaluationsDBState interfEvaluationsDBState;

  /// length of packed MPI buffers containing vars, vars/set, response,
  /// and PRPair
  IntArray messageLengths;

  /// track use of initialize_mapping() and finalize_mapping()
  bool mappingInitialized;

  /// class member reference to the problem description database
  /** Iterator and Model cannot use a shallow copy of ProblemDescDB
      due to circular destruction dependency (reference counts can't
      get to 0), since ProblemDescDB contains {iterator,model}List. */
  ProblemDescDB& probDescDB;

  /// class member reference to the parallel library
  ParallelLibrary& parallelLib;

  /// the ParallelConfiguration node used by this Model instance
  ParConfigLIter modelPCIter;

  /// the component parallelism mode: NO_PARALLEL_MODE, SURROGATE_MODEL_MODE,
  // TRUTH_MODEL_MODE, SUB_MODEL_MODE, INTERFACE_MODE, OPTIONAL_INTERFACE_MODE
  short componentParallelMode;

  /// flags asynch evaluations (local or distributed)
  bool asynchEvalFlag;

  /// capacity for concurrent evaluations supported by the Model
  int evaluationCapacity;

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;

  /*
  /// array of IntSet's, each containing the set of allowable integer
  /// values corresponding to a discrete design integer set variable
  IntSetArray discreteDesignSetIntValues;
  /// array of IntSet's, each containing the set of allowable integer
  /// values corresponding to a discrete design string set variable
  StringSetArray discreteDesignSetStringValues;
  /// array of RealSet's, each containing the set of allowable real
  /// values corresponding to a discrete design real set variable
  RealSetArray discreteDesignSetRealValues;

  /// array of IntSet's, each containing the set of allowable integer
  /// values corresponding to a discrete state integer set variable
  IntSetArray discreteStateSetIntValues;
  /// array of IntSet's, each containing the set of allowable integer
  /// values corresponding to a discrete state string set variable
  StringSetArray discreteStateSetStringValues;
  /// array of RealSet's, each containing the set of allowable real
  /// values corresponding to a discrete state real set variable
  RealSetArray discreteStateSetRealValues;
  */

  /// the multivariate random variable distribution (in probability space
  /// corresponding to currentVariables)
  Pecos::MultivariateDistribution mvDist;

  /// array of flags (one per primary function) for switching the
  /// sense to maximize the primary function (default is minimize)
  BoolDeque primaryRespFnSense;
  /// primary response function weightings (either weights for
  /// multiobjective optimization or weighted least squares)
  RealVector primaryRespFnWts;

  /// whether to perform hierarchical evalID tagging of params/results
  bool hierarchicalTagging;

  /// user-provided scaling data from the problem DB, possibly
  /// modified by Recasting
  ScalingOptions scalingOpts;

  /// cached evalTag Prefix from parents to use at evaluate time
  String evalTagPrefix;

  /// reference to the global evaluation database
  EvaluationStore &evaluationsDB;

private:
 
  //
  //- Heading: Member functions
  //

  // *** BASE MODEL (BROADLY USED, OPERATES ON BASE DATA)

  /// evaluate numerical gradients using finite differences.  This
  /// routine is selected with "method_source dakota" (the default
  /// method_source) in the numerical gradient specification.
  int estimate_derivatives(const ShortArray& map_asv,
			   const ShortArray& fd_grad_asv,
			   const ShortArray& fd_hess_asv,
			   const ShortArray& quasi_hess_asv,
			   const ActiveSet& original_set,
			   const bool asynch_flag);

  /// combine results from an array of finite difference response
  /// objects (fd_grad_responses) into a single response (new_response)
  void synchronize_derivatives(const Variables& vars,
			       const IntResponseMap& fd_responses, 
			       Response& new_response,
			       const ShortArray& fd_grad_asv,
			       const ShortArray& fd_hess_asv,
			       const ShortArray& quasi_hess_asv,
			       const ActiveSet& original_set);

  /// overlay results to update a response object
  void update_response(const Variables& vars, Response& new_response,
		       const ShortArray& fd_grad_asv,
		       const ShortArray& fd_hess_asv,
		       const ShortArray& quasi_hess_asv,
		       const ActiveSet& original_set,
		       Response& initial_map_response,
		       const RealMatrix& new_fn_grads,
		       const RealSymMatrixArray& new_fn_hessians);

  /// perform quasi-Newton Hessian updates
  void update_quasi_hessians(const Variables& vars, Response& new_response,
			     const ActiveSet& original_set);

  /// Coordinates usage of estimate_derivatives() calls based on asv_in
  bool manage_asv(const ActiveSet& original_set, ShortArray& map_asv_out, 
		  ShortArray& fd_grad_asv_out, ShortArray& fd_hess_asv_out,
		  ShortArray& quasi_hess_asv_out);

  /// function to determine initial finite difference h (before step
  /// length adjustment) based on type of step desired
  Real initialize_h(Real x_j, Real lb_j, Real ub_j, Real step_size,
		    String step_type) const;
  /// function returning finite-difference step size (affected by bounds)
  Real FDstep1(Real x0_j, Real lb_j, Real ub_j, Real h_mag);
  /// function returning second central-difference step size (affected
  /// by bounds)
  Real FDstep2(Real x0_j, Real lb_j, Real ub_j, Real h);

  //
  //- Heading: Data
  //

  /// evaluation counter for top-level evaluate() and evaluate_nowait()
  /// calls.  Differs from lower level counters in case of numerical
  /// derivative estimation (several lower level evaluations are
  /// assimilated into a single higher level evaluation)
  int modelEvalCntr;

  /// flags presence of estimated derivatives within a set of calls to
  /// evaluate_nowait()
  bool estDerivsFlag;
  /// flags finite-difference step size adjusted by bounds
  bool shortStep;

  /// map<> used for tracking modelPCIter instances using depth of parallelism
  /// level and max evaluation concurrency as the lookup keys
  std::map<SizetIntPair, ParConfigLIter> modelPCIterMap;

  /// flag for determining need to bcast the max concurrency from
  /// init_communicators(); set from IteratorScheduler::init_iterator()
  bool initCommsBcastFlag;

  /// flag for posting of graphics data within evaluate()
  /// (automatic graphics posting in the model as opposed to
  /// graphics posting at the strategy level)
  bool modelAutoGraphicsFlag;

  /// history of vars populated in evaluate_nowait() and used in
  /// synchronize().
  IntVariablesMap varsMap;
  /// if estimate_derivatives() is used, transfers ASVs from
  /// evaluate_nowait() to synchronize()
  std::list<ShortArray> asvList;
  /// if estimate_derivatives() is used, transfers ActiveSets from
  /// evaluate_nowait() to synchronize()
  std::list<ActiveSet> setList;
  /// transfers initial_map flag values from estimate_derivatives() to
  /// synchronize_derivatives()
  BoolList initialMapList;
  /// transfers db_capture flag values from estimate_derivatives() to
  /// synchronize_derivatives()
  BoolList dbCaptureList;
  /// transfers database captures from estimate_derivatives() to
  /// synchronize_derivatives()
  ResponseList dbResponseList;
  /// transfers deltas from estimate_derivatives() to synchronize_derivatives()
  RealList deltaList;

  /// tracks the number of evaluations used within estimate_derivatives().
  /// Used in synchronize() as a key for combining finite difference
  /// responses into numerical gradients.
  IntIntMap numFDEvalsMap;
  /// maps from the raw evaluation ids returned by derived_synchronize() and
  /// derived_synchronize_nowait() to the corresponding modelEvalCntr id.
  /// Used for rekeying responseMap.
  IntIntMap rawEvalIdMap;

  /// previous parameter vectors used in computing s for quasi-Newton updates
  RealVectorArray xPrev;
  /// previous gradient vectors used in computing y for quasi-Newton updates
  RealMatrix fnGradsPrev;

  /// quasi-Newton Hessian approximations
  RealSymMatrixArray quasiHessians;
  /// number of quasi-Newton Hessian updates applied
  SizetArray numQuasiUpdates;

  /// used to return a map of responses for asynchronous evaluations in final
  /// concatenated form.  The similar map in Interface contains raw responses.
  IntResponseMap responseMap;
  /// caching of responses returned by derived_synchronize{,_nowait}()
  /// but not matched within current rawEvalIdMap
  IntResponseMap cachedResponseMap;
  /// used to cache the data returned from derived_synchronize_nowait() prior
  /// to sequential input into the graphics
  IntResponseMap graphicsRespMap;
  /// used to collect sub-models for subordinate_models()
  ModelList modelList;
  /// a key indicating which models within a model recursion involve recasting
  BoolDeque recastFlags;

  /// pointer to the letter (initialized only for the envelope)
  std::shared_ptr<Model> modelRep;

  /// the last used model ID number for on-the-fly instantiations
  /// (increment before each use)
  static size_t noSpecIdNum;
};


inline int Model::evaluation_id() const
{ return modelEvalCntr; }


inline bool Model::recastings() const
{ return !recastFlags.empty(); }


inline Pecos::MultivariateDistribution& Model::multivariate_distribution()
{ return mvDist; }


inline const Pecos::MultivariateDistribution& Model::
multivariate_distribution() const
{ return mvDist; }


inline const Variables& Model::current_variables() const
{ return currentVariables; }


inline Variables& Model::current_variables()
{ return currentVariables; }


inline const Constraints& Model::user_defined_constraints() const
{
  return userDefinedConstraints;
}


inline Constraints& Model::user_defined_constraints()
{
  return userDefinedConstraints;
}


inline const Response& Model::current_response() const
{ return currentResponse; }


inline Response& Model::current_response()
{ return currentResponse; }


inline ProblemDescDB& Model::problem_description_db() const
{ return probDescDB; }


inline ParallelLibrary& Model::parallel_library() const
{ return parallelLib; }


inline const String& Model::model_type() const
{ return modelType; }


inline const String& Model::surrogate_type() const
{ return surrogateType; }


inline const String& Model::model_id() const
{ return modelId; }


inline size_t Model::num_secondary_fns() const
{
  return
    ModelUtils::num_nonlinear_ineq_constraints(*this) + 
    ModelUtils::num_nonlinear_eq_constraints(*this);
}


inline size_t Model::num_primary_fns() const
{
  return
    current_response().num_functions() - num_secondary_fns();
}


inline const String& Model::gradient_type() const
{ return gradientType; }


inline const String& Model::method_source() const
{ return methodSource; }


inline const String& Model::interval_type() const
{ return intervalType; }


inline bool Model::ignore_bounds() const
{ return ignoreBounds; }


inline bool Model::central_hess() const
{ return centralHess; }


inline const RealVector& Model::fd_gradient_step_size() const
{ return fdGradStepSize; }


inline const String& Model::fd_gradient_step_type() const
{ return fdGradStepType; }


inline const IntSet& Model::gradient_id_analytic() const
{ return gradIdAnalytic; }


inline const IntSet& Model::gradient_id_numerical() const
{ return gradIdNumerical; }


inline const String& Model::hessian_type() const
{ return hessianType; }


inline const String& Model::quasi_hessian_type() const
{ return quasiHessType; }


inline const RealVector& Model::fd_hessian_by_grad_step_size() const
{ return fdHessByGradStepSize; }


inline const RealVector& Model::fd_hessian_by_fn_step_size() const
{ return fdHessByFnStepSize; }


inline const String& Model::fd_hessian_step_type() const
{ return fdHessStepType; }


inline const IntSet& Model::hessian_id_analytic() const
{ return hessIdAnalytic; }


inline const IntSet& Model::hessian_id_numerical() const
{ return hessIdNumerical; }


inline const IntSet& Model::hessian_id_quasi() const
{ return hessIdQuasi; }


inline void Model::primary_response_fn_sense(const BoolDeque& sense)
{
  primaryRespFnSense = sense;
}


inline const BoolDeque& Model::primary_response_fn_sense() const
{ return primaryRespFnSense; }


inline const RealVector& Model::primary_response_fn_weights() const
{ return primaryRespFnWts; }


inline const ScalingOptions& Model::scaling_options() const
{ return scalingOpts; }


inline short Model::primary_fn_type() const
{ 
  return
    currentResponse.shared_data().primary_fn_type(); 
}

inline void Model::primary_fn_type(short type)
{
  currentResponse.shared_data().primary_fn_type(type);
}


inline bool Model::derivative_estimation()
{
  return
    ( (gradientType == "numerical" || gradientType == "mixed") ||
      (hessianType == "numerical" || hessianType == "mixed" ||
       hessianType == "quasi") );
}


inline void Model::supports_derivative_estimation(bool sed_flag)
{
  supportsEstimDerivs = sed_flag;
}


inline void Model::init_comms_bcast_flag(bool icb_flag) 
{
  initCommsBcastFlag = icb_flag;
}


inline int Model::evaluation_capacity() const
{ return evaluationCapacity; }


inline bool Model::asynch_flag() const
{ return asynchEvalFlag; }


inline void Model::asynch_flag(const bool flag)
{
  asynchEvalFlag = flag;
}


inline short Model::output_level() const
{ return outputLevel; }


inline void Model::output_level(const short level)
{
  outputLevel = level;
}


inline const IntArray& Model::message_lengths() const
{ return messageLengths; }


inline bool Model::mapping_initialized() const
{ return mappingInitialized; }


inline void Model::parallel_configuration_iterator(ParConfigLIter pc_iter)
{
  modelPCIter = pc_iter;
}


inline ParConfigLIter Model::parallel_configuration_iterator() const
{ return modelPCIter; }


inline void Model::auto_graphics(const bool flag)
{
  modelAutoGraphicsFlag = flag;
}

inline bool Model::auto_graphics() const
{
  return modelAutoGraphicsFlag;
}


inline SSCIter Model::max_string(const StringSet& ss)
{
  SSCIter ss_it = ss.begin(),  max_it = ss_it;  ++ss_it;
  for (; ss_it!=ss.end(); ++ss_it)
    if (ss_it->size() > max_it->size())
      max_it = ss_it;
  return max_it;
}


inline SRMCIter Model::max_string(const StringRealMap& srm)
{
  SRMCIter srm_it = srm.begin(),  max_it = srm_it;  ++srm_it;
  for (; srm_it!=srm.end(); ++srm_it)
    if (srm_it->first.size() > max_it->first.size())
      max_it = srm_it;
  return max_it;
}


inline IntResponseMap& Model::response_map()
{ return responseMap; }


/*
inline void Model::
rekey_response_map(const IntResponseMap& resp_map, IntIntMap& id_map,
		   IntResponseMap& resp_map_rekey,
		   IntResponseMap& cached_resp_map, bool deep_copy)
{
  // rekey registered evals
  resp_map_rekey.clear();

  // process incoming resp_map against remaining id_map
  IntIntMIter id_it; IntRespMCIter r_cit;
  for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit) {
    id_it = id_map.find(r_cit->first); // Note: no iterator hint API
    if (id_it != id_map.end()) {
      resp_map_rekey[id_it->second] = (deep_copy) ?
	r_cit->second.copy() : r_cit->second;
      id_map.erase(id_it);
    }
    // insert unmatched resp_map jobs into cache (may be from another Model
    // using a shared Interface instance).  Neither deep copy nor rekey are
    // performed until id is matched above (on a subsequent pass).
    else
      // Approach 1: the resp_map record is duplicated, not migrated, due to
      // const ref --> rely on subsequent clearing of resp_map on reentry
      cached_resp_map[r_cit->first] = r_cit->second;
      // Approach 2: virtual fn approach could accomplish a migration and
      // avoid duplication, but it lacks level clarity (SimulationModel:
      // augment its own cache or delegate to userDefinedInterface?) 
      //cache_unmatched_response(r_cit->first); // virtual fn
      // Approach 3: resolve level clarity by passing a meta-object in a
      // template.  Consistency: use meta-object to synchronize and cache,
      // requiring renaming of Interface fns and adding nowait variant.
      //meta_object.cache_unmatched_response(r_cit->first); // level clarity
  }
}
*/


template <typename MetaType> void Model::
rekey_response_map(MetaType& meta_object, IntIntMapArray& id_maps,
		   IntResponseMapArray& resp_maps_rekey, bool deep_copy)
{
  // rekey the IntResponseMap evals matched in id_maps, else move to cache
  IntResponseMap& orig_resp_map = meta_object.response_map();
  IntRespMIter r_it = orig_resp_map.begin();
  size_t i, num_maps = id_maps.size(), resp_eval_id;
  std::vector<IntIntMIter> id_iters(num_maps);  IntIntMIter id_ite;
  for (i=0; i<num_maps; ++i)
    id_iters[i] = id_maps[i].begin();
  resp_maps_rekey.clear();  resp_maps_rekey.resize(num_maps);

  // Single traversal of orig_resp_map and all id_maps
  bool found, active_id_maps = true;
  while (r_it != orig_resp_map.end() && active_id_maps) {
    resp_eval_id = r_it->first;
    found = active_id_maps = false;
    for (i=0; i<num_maps; ++i) {
      IntIntMap&   id_map = id_maps[i];
      IntIntMIter& id_it  = id_iters[i];  id_ite = id_map.end();
      while (id_it != id_ite && id_it->first <  resp_eval_id) ++id_it;
      if    (id_it != id_ite && id_it->first == resp_eval_id) {
	// process match and increment both iterators
	Response& resp = r_it->second;
	resp_maps_rekey[i][id_it->second] = (deep_copy) ? resp.copy() : resp;
	if (evaluations_db_state(meta_object) == EvaluationsDBState::ACTIVE)
	  asynch_eval_store(meta_object, id_it->first, resp);
	id_map.erase(id_it++);      // postfix increment
	orig_resp_map.erase(r_it++);// postfix increment 
	found = true;
      }
      //if (found) break;// interferes with active_id_maps and only defers ++it
      if (id_it != id_ite) active_id_maps = true;
    }
    if (!found) ++r_it; // else already advanced
  }

  // insert unmatched resp_map jobs into cache (may be from another Model
  // using a shared Interface instance).  Neither deep copy nor rekey are
  // performed until id is matched above (on a subsequent pass).
  if (!orig_resp_map.empty())
    meta_object.cache_unmatched_responses();
}


template <typename MetaType> void Model::
rekey_synch(MetaType& meta_object, bool block, IntIntMapArray& id_maps,
	    IntResponseMapArray& resp_maps_rekey, bool deep_copy)
{
  if (block) meta_object.synchronize();
  else       meta_object.synchronize_nowait();
  rekey_response_map(meta_object, id_maps, resp_maps_rekey, deep_copy);
}


template <typename MetaType> void Model::
rekey_response_map(MetaType& meta_object, IntIntMap& id_map,
		   IntResponseMap& resp_map_rekey, bool deep_copy)
{
  // rekey the IntResponseMap evals matched in id_map, else move to cache
  IntResponseMap& orig_resp_map = meta_object.response_map();
  IntRespMIter r_it = orig_resp_map.begin();
  IntIntMIter id_it =        id_map.begin();
  size_t i, orig_eval_id, resp_eval_id;
  resp_map_rekey.clear();

  // Single traversal of orig_resp_map and id_map
  while (id_it != id_map.end() && r_it != orig_resp_map.end()) {
    orig_eval_id = id_it->first;
    resp_eval_id =  r_it->first;
    if      (orig_eval_id < resp_eval_id) ++id_it;
    else if (orig_eval_id > resp_eval_id)  ++r_it;
    else { // equal: process match and increment both iterators
      Response& resp = r_it->second;
      resp_map_rekey[id_it->second] = (deep_copy) ? resp.copy() : resp;
      if (evaluations_db_state(meta_object) == EvaluationsDBState::ACTIVE)
	asynch_eval_store(meta_object, orig_eval_id, resp);
      id_map.erase(id_it++);      // postfix increment avoids iter invalidation
      orig_resp_map.erase(r_it++);// postfix increment avoids iter invalidation
    }
  }

  // insert unmatched resp_map jobs into cache (may be from another Model
  // using a shared Interface instance).  Neither deep copy nor rekey are
  // performed until id is matched above (on a subsequent pass).
  if (!orig_resp_map.empty())
    meta_object.cache_unmatched_responses();
}


template <typename MetaType> void Model::
rekey_synch(MetaType& meta_object, bool block, IntIntMap& id_map,
	    IntResponseMap& resp_map_rekey, bool deep_copy)
{
  if (block) meta_object.synchronize();
  else       meta_object.synchronize_nowait();
  rekey_response_map(meta_object, id_map, resp_map_rekey, deep_copy);
}


/// global comparison function for Model
inline bool model_id_compare(const Model& model, const void* id)
{ return ( *(const String*)id == model.model_id() ); }


} // namespace Dakota

#endif
