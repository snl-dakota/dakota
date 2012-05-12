/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Model
//- Description: The model to be iterated.  Contains Variables, 
//-              Interface, and Response objects.
//- Owner:       Mike Eldred
//- Version: $Id: DakotaModel.H 7029 2010-10-22 00:17:02Z mseldre $

#ifndef DAKOTA_MODEL_H
#define DAKOTA_MODEL_H

#include "data_types.h"
#include "DakotaVariables.H"
#include "DakotaConstraints.H"
//#include "DakotaInterface.H"
#include "DakotaResponse.H"
#include "DistributionParams.hpp"

// likely need more general _WIN32 here
#ifdef __MINGW32__
#undef interface
#endif

namespace Pecos { class SurrogateData; /* forward declarations */ }

namespace Dakota {

// forward declarations
class ProblemDescDB;
class ParallelLibrary;
class Approximation;
class DiscrepancyCorrection;


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
  /// standard constructor for envelope
  Model(ProblemDescDB& problem_db);
  /// copy constructor
  Model(const Model& model);

  /// destructor
  virtual ~Model();

  /// assignment operator
  Model operator=(const Model& model);

  //
  //- Heading: Virtual functions
  //

  /// return the sub-iterator in nested and surrogate models
  virtual Iterator& subordinate_iterator();
  /// return a single sub-model defined from subModel in nested and recast
  /// models and truth_model() in surrogate models; used for a directed
  /// dive through model recursions that may bypass some components.
  virtual Model& subordinate_model();
  /// return the approximation sub-model in surrogate models
  virtual Model& surrogate_model();
  /// return the truth sub-model in surrogate models
  virtual Model& truth_model();
  /// portion of subordinate_models()() specific to derived model classes
  virtual void derived_subordinate_models(ModelList& ml, bool recurse_flag);
  /// propagate vars/labels/bounds/targets from the bottom up
  virtual void update_from_subordinate_model(bool recurse_flag = true);
  /// return the interface employed by the derived model class, if present:
  /// SingleModel::userDefinedInterface, DataFitSurrModel::approxInterface,
  /// or NestedModel::optionalInterface
  virtual Interface& interface();

  /// set the relative weightings for multiple objective functions or least
  /// squares terms
  virtual void primary_response_fn_weights(const RealVector& wts, 
					   bool recurse_flag = true);

  /// set the (currently active) surrogate function index set
  virtual void surrogate_function_indices(const IntSet& surr_fn_indices);

  /// build a new SurrogateModel approximation
  virtual void build_approximation();
  /// build a new SurrogateModel approximation using/enforcing
  /// response at vars
  virtual bool build_approximation(const Variables& vars,
				   const IntResponsePair& response_pr);


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

  /// append to the existing approximation data within a surrogate
  /// based on data updates propagated elsewhere
  virtual void append_approximation(bool rebuild_flag);
  /// append a single point to an existing surrogate's data
  virtual void append_approximation(const Variables& vars, 
				    const IntResponsePair& response_pr,
				    bool rebuild_flag);
  /// append multiple points to an existing surrogate's data
  virtual void append_approximation(const VariablesArray& vars_array,
				    const IntResponseMap& resp_map,
				    bool rebuild_flag);

  /// remove the previous data set addition to a surrogate (e.g., due
  /// to a previous append_approximation() call); flag manages storing
  /// of surrogate data for use in a subsequent restore_approximation()
  virtual void pop_approximation(bool save_surr_data);

  /// restore a previous approximation data state within a surrogate
  virtual void restore_approximation();
  /// query for whether a trial increment is restorable within a surrogate
  virtual bool restore_available();

  /// finalize an approximation by applying all previous trial increments
  virtual void finalize_approximation();

  /// move the current approximation into storage for later combination
  virtual void store_approximation();
  /// combine the current approximation with one previously stored
  virtual void combine_approximation(short corr_type);

  // retrieve the variables used to build a surrogate model
  //virtual const VariablesArray build_variables() const;
  // retrieve the responses used to build a surrogate model
  //virtual const ResponseArray build_responses() const;

  /// determine whether a surrogate model rebuild should be forced
  /// based on changes in the inactive data
  virtual bool force_rebuild();

  /// retrieve the set of Approximations within a DataFitSurrModel
  virtual std::vector<Approximation>& approximations();
  /// retrieve the approximation data from a particular Approximation
  /// instance within a DataFitSurrModel
  virtual const Pecos::SurrogateData& approximation_data(size_t index);

  /// retrieve the approximation coefficients from each Approximation
  /// within a DataFitSurrModel
  virtual const RealVectorArray& approximation_coefficients();
  /// set the approximation coefficients for each Approximation within
  /// a DataFitSurrModel
  virtual void approximation_coefficients(const RealVectorArray& approx_coeffs);
  /// retrieve the approximation variances from each
  /// Approximation within a DataFitSurrModel
  virtual const RealVector& approximation_variances(
    const RealVector& c_vars);

  /// set response computation mode used in SurrogateModels for
  /// forming currentResponse
  virtual void surrogate_response_mode(short mode);
  /// return response computation mode used in SurrogateModels for
  /// forming currentResponse
  virtual short surrogate_response_mode() const;

  /// return the DiscrepancyCorrection object used by SurrogateModels
  virtual DiscrepancyCorrection& discrepancy_correction();

  /// update component parallel mode for supporting parallelism in a model's
  /// interface component, sub-model component, or neither component
  /// [componentParallelMode = 0 (none), 1
  /// (INTERFACE/APPROX_INTERFACE/OPTIONAL_INTERFACE/LF_MODEL/SURROGATE_MODEL),
  /// or 2 (SUB_MODEL/ACTUAL_MODEL/HF_MODEL/TRUTH_MODEL)].
  virtual void component_parallel_mode(short mode);

  /// return derived model synchronization setting
  virtual String local_eval_synchronization();
  /// return derived model asynchronous evaluation concurrency
  virtual int local_eval_concurrency();

  /// Service job requests received from the master.  Completes when
  /// a termination message is received from stop_servers().
  virtual void serve();
  /// Executed by the master to terminate all server operations for a
  /// particular model when iteration on the model is complete.
  virtual void stop_servers();

  /// Return a flag indicating the combination of multiprocessor
  /// evaluations and a dedicated master iterator scheduling.  Used
  /// in synchronous compute_response functions to prevent the error
  /// of trying to run a multiprocessor job on the master.
  virtual bool derived_master_overload() const;

  /// update the Model's inactive view based on higher level (nested) context
  virtual void inactive_view(short view, bool recurse_flag = true);

  /// return the interface identifier
  virtual const String& interface_id() const;
  /// Return the value of the evaluation id counter for the Model
  virtual int evaluation_id() const;
  /// Indicates the usage of an evaluation cache by the Model
  virtual bool evaluation_cache() const;

  /// Set the reference points for the evaluation counters within the Model
  virtual void set_evaluation_reference();
  /// Request fine-grained evaluation reporting within the Model
  virtual void fine_grained_evaluation_counters();
  /// Print an evaluation summary for the Model
  virtual void print_evaluation_summary(std::ostream& s,
					bool minimal_header = false,
					bool relative_count = true) const;

  //
  //- Heading: Member functions
  //

  /// return the sub-models in nested and surrogate models
  ModelList& subordinate_models(bool recurse_flag = true);

  // TO DO: synchronous compute_response() should return int codes for
  // algorithm-specific mitigation of captured failures

  /// Compute the Response at currentVariables (default ActiveSet).
  void compute_response();
  /// Compute the Response at currentVariables (specified ActiveSet).
  void compute_response(const ActiveSet& set);

  /// Spawn an asynchronous job (or jobs) that computes the value of the 
  /// Response at currentVariables (default ActiveSet).
  void asynch_compute_response();
  /// Spawn an asynchronous job (or jobs) that computes the value of the 
  /// Response at currentVariables (specified ActiveSet).
  void asynch_compute_response(const ActiveSet& set);

  // TO DO: for asynch_compute_response(), add access fns for ShortArray/List
  // return codes for algorithm-specific mitigation of captured failures.

  /// Execute a blocking scheduling algorithm to collect the
  /// complete set of results from a group of asynchronous evaluations.
  const IntResponseMap& synchronize();
  /// Execute a nonblocking scheduling algorithm to collect all
  /// available results from a group of asynchronous evaluations.
  const IntResponseMap& synchronize_nowait();

  /// allocate communicator partitions for a model and store
  /// configuration in modelPCIterMap
  void init_communicators(int max_iterator_concurrency,
			  bool recurse_flag = true);
  /// for cases where init_communicators() will not be called,
  /// modify some default settings to behave properly in serial.
  void init_serial();
  /// set active parallel configuration for the model (set modelPCIter
  /// from modelPCIterMap)
  void set_communicators(int max_iterator_concurrency,
			 bool recurse_flag = true);
  /// deallocate communicator partitions for a model
  void free_communicators(int max_iterator_concurrency,
			  bool recurse_flag = true);

  /// called from Strategy::init_iterator() for iteratorComm rank 0 to
  /// terminate serve_configurations() on other iteratorComm processors
  void stop_configurations();
  /// called from Strategy::init_iterator() for iteratorComm rank != 0
  /// to balance init_communicators() calls on iteratorComm rank 0
  int serve_configurations();

  /// estimate messageLengths for a model
  void estimate_message_lengths();

  //
  //- Heading: Set and Inquire functions
  //
 
  /// replaces existing letter with a new one
  void assign_rep(Model* model_rep, bool ref_count_incr = true);

  // VARIABLES

  size_t  tv()  const; ///< returns total number of vars
  size_t  cv()  const; ///< returns number of active continuous variables
  size_t div()  const; ///< returns number of active discrete integer vars
  size_t drv()  const; ///< returns number of active discrete real vars
  size_t icv()  const; ///< returns number of inactive continuous variables
  size_t idiv() const; ///< returns number of inactive discrete integer vars
  size_t idrv() const; ///< returns number of inactive discrete real vars
  size_t acv()  const; ///< returns total number of continuous variables
  size_t adiv() const; ///< returns total number of discrete integer vars
  size_t adrv() const; ///< returns total number of discrete real vars

  /// set the active variables in currentVariables
  void active_variables(const Variables& vars);

  /// return the active continuous variables from currentVariables
  const RealVector& continuous_variables() const;
  /// set the active continuous variables in currentVariables
  void continuous_variables(const RealVector& c_vars);
  /// set an active continuous variable in currentVariables
  void continuous_variable(const Real& c_var, const size_t& i);
  /// return the active discrete integer variables from currentVariables
  const IntVector& discrete_int_variables() const;
  /// set the active discrete integer variables in currentVariables
  void discrete_int_variables(const IntVector& d_vars);
  /// set an active discrete integer variable in currentVariables
  void discrete_int_variable(const int& d_var, const size_t& i);
  /// return the active discrete real variables from currentVariables
  const RealVector& discrete_real_variables() const;
  /// set the active discrete real variables in currentVariables
  void discrete_real_variables(const RealVector& d_vars);
  /// set an active discrete real variable in currentVariables
  void discrete_real_variable(const Real& d_var, const size_t& i);

  /// return the active continuous variable types from currentVariables
  UShortMultiArrayConstView continuous_variable_types()    const;
  /// return the active discrete variable types from currentVariables
  UShortMultiArrayConstView discrete_int_variable_types()  const;
  /// return the active discrete variable types from currentVariables
  UShortMultiArrayConstView discrete_real_variable_types() const;
  /// return the active continuous variable identifiers from currentVariables
  SizetMultiArrayConstView  continuous_variable_ids()      const;

  /// return the inactive continuous variables in currentVariables
  const RealVector& inactive_continuous_variables() const;
  /// set the inactive continuous variables in currentVariables
  void inactive_continuous_variables(const RealVector& i_c_vars);
  /// return the inactive discrete variables in currentVariables
  const IntVector& inactive_discrete_int_variables() const;
  /// set the inactive discrete variables in currentVariables
  void inactive_discrete_int_variables(const IntVector& i_d_vars);
  /// return the inactive discrete variables in currentVariables
  const RealVector& inactive_discrete_real_variables() const;
  /// set the inactive discrete variables in currentVariables
  void inactive_discrete_real_variables(const RealVector& i_d_vars);

  /// return the inactive continuous variable types from currentVariables
  UShortMultiArrayConstView inactive_continuous_variable_types() const;
  /// return the inactive continuous variable identifiers from currentVariables
  SizetMultiArrayConstView inactive_continuous_variable_ids() const;

  /// return all continuous variables in currentVariables
  const RealVector& all_continuous_variables() const;
  /// set all continuous variables in currentVariables
  void all_continuous_variables(const RealVector& a_c_vars);
  /// set a variable within the all continuous variables in currentVariables
  void all_continuous_variable(const Real& a_c_var, const size_t& i);
  /// return all discrete variables in currentVariables
  const IntVector& all_discrete_int_variables() const;
  /// set all discrete variables in currentVariables
  void all_discrete_int_variables(const IntVector& a_d_vars);
  /// set a variable within the all discrete variables in currentVariables
  void all_discrete_int_variable(const int& a_d_var, const size_t& i);
  /// return all discrete variables in currentVariables
  const RealVector& all_discrete_real_variables() const;
  /// set all discrete variables in currentVariables
  void all_discrete_real_variables(const RealVector& a_d_vars);
  /// set a variable within the all discrete variables in currentVariables
  void all_discrete_real_variable(const Real& a_d_var, const size_t& i);

  /// return all continuous variable types from currentVariables
  UShortMultiArrayConstView all_continuous_variable_types()    const;
  /// return all discrete variable types from currentVariables
  UShortMultiArrayConstView all_discrete_int_variable_types()  const;
  /// return all discrete variable types from currentVariables
  UShortMultiArrayConstView all_discrete_real_variable_types() const;
  /// return all continuous variable identifiers from currentVariables
  SizetMultiArrayConstView  all_continuous_variable_ids()      const;

  /// return the sets of values available for each of the discrete
  /// design set integer variables
  const IntSetArray& discrete_design_set_int_values() const;
  /// define the sets of values available for each of the discrete
  /// design set integer variables
  void discrete_design_set_int_values(const IntSetArray& isa);
  /// return the sets of values available for each of the discrete
  /// design set integer variables
  const RealSetArray& discrete_design_set_real_values() const;
  /// define the sets of values available for each of the discrete
  /// design set integer variables
  void discrete_design_set_real_values(const RealSetArray& rsa);

  /// return distribParams
  Pecos::DistributionParams& distribution_parameters();
  /// return distribParams
  const Pecos::DistributionParams& distribution_parameters() const;
  /// set distribParams
  void distribution_parameters(const Pecos::DistributionParams& dp);

  /// return the sets of values available for each of the discrete
  /// state set integer variables
  const IntSetArray& discrete_state_set_int_values() const;
  /// define the sets of values available for each of the discrete
  /// state set integer variables
  void discrete_state_set_int_values(const IntSetArray& isa);
  /// return the sets of values available for each of the discrete
  /// state set integer variables
  const RealSetArray& discrete_state_set_real_values() const;
  /// define the sets of values available for each of the discrete
  /// state set integer variables
  void discrete_state_set_real_values(const RealSetArray& rsa);

  // LABELS and TAGS

  /// return the active continuous variable labels from currentVariables
  StringMultiArrayConstView continuous_variable_labels() const;
  /// set the active continuous variable labels in currentVariables
  void continuous_variable_labels(StringMultiArrayConstView c_v_labels);
  /// return the active discrete variable labels from currentVariables
  StringMultiArrayConstView discrete_int_variable_labels()   const;
  /// set the active discrete variable labels in currentVariables
  void discrete_int_variable_labels(StringMultiArrayConstView d_v_labels);
  /// return the active discrete variable labels from currentVariables
  StringMultiArrayConstView discrete_real_variable_labels()   const;
  /// set the active discrete variable labels in currentVariables
  void discrete_real_variable_labels(StringMultiArrayConstView d_v_labels);

  /// return the inactive continuous variable labels in currentVariables
  StringMultiArrayConstView inactive_continuous_variable_labels() const;
  /// set the inactive continuous variable labels in currentVariables
  void inactive_continuous_variable_labels(
    StringMultiArrayConstView i_c_v_labels);
  /// return the inactive discrete variable labels in currentVariables
  StringMultiArrayConstView inactive_discrete_int_variable_labels() const;
  /// set the inactive discrete variable labels in currentVariables
  void inactive_discrete_int_variable_labels(
    StringMultiArrayConstView i_d_v_labels);
  /// return the inactive discrete variable labels in currentVariables
  StringMultiArrayConstView inactive_discrete_real_variable_labels() const;
  /// set the inactive discrete variable labels in currentVariables
  void inactive_discrete_real_variable_labels(
    StringMultiArrayConstView i_d_v_labels);

  /// return all continuous variable labels in currentVariables
  StringMultiArrayConstView all_continuous_variable_labels() const;
  /// set all continuous variable labels in currentVariables
  void all_continuous_variable_labels(StringMultiArrayConstView a_c_v_labels);
  /// set a label within the all continuous labels in currentVariables
  void all_continuous_variable_label(const String& a_c_v_label,const size_t& i);
  /// return all discrete variable labels in currentVariables
  StringMultiArrayConstView all_discrete_int_variable_labels() const;
  /// set all discrete variable labels in currentVariables
  void all_discrete_int_variable_labels(StringMultiArrayConstView a_d_v_labels);
  /// set a label within the all discrete labels in currentVariables
  void all_discrete_int_variable_label(const String& a_d_v_label,
				       const size_t& i);
  /// return all discrete variable labels in currentVariables
  StringMultiArrayConstView all_discrete_real_variable_labels() const;
  /// set all discrete variable labels in currentVariables
  void all_discrete_real_variable_labels(
    StringMultiArrayConstView a_d_v_labels);
  /// set a label within the all discrete labels in currentVariables
  void all_discrete_real_variable_label(const String& a_d_v_label,
					const size_t& i);

  /// return the response labels from currentResponse
  const StringArray& response_labels() const;
  /// set the response labels in currentResponse
  void response_labels(const StringArray& resp_labels);

  // BOUNDS

  /// return the active continuous lower bounds from userDefinedConstraints
  const RealVector& continuous_lower_bounds() const;
  /// set the active continuous lower bounds in userDefinedConstraints
  void continuous_lower_bounds(const RealVector& c_l_bnds);
  /// return the active continuous upper bounds from userDefinedConstraints
  const RealVector& continuous_upper_bounds() const;
  /// set the active continuous upper bounds in userDefinedConstraints
  void continuous_upper_bounds(const RealVector& c_u_bnds);
  /// return the active discrete lower bounds from userDefinedConstraints
  const IntVector& discrete_int_lower_bounds() const;
  /// set the active discrete lower bounds in userDefinedConstraints
  void discrete_int_lower_bounds(const IntVector& d_l_bnds);
  /// return the active discrete upper bounds from userDefinedConstraints
  const IntVector& discrete_int_upper_bounds() const;
  /// set the active discrete upper bounds in userDefinedConstraints
  void discrete_int_upper_bounds(const IntVector& d_u_bnds);
  /// return the active discrete lower bounds from userDefinedConstraints
  const RealVector& discrete_real_lower_bounds() const;
  /// set the active discrete lower bounds in userDefinedConstraints
  void discrete_real_lower_bounds(const RealVector& d_l_bnds);
  /// return the active discrete upper bounds from userDefinedConstraints
  const RealVector& discrete_real_upper_bounds() const;
  /// set the active discrete upper bounds in userDefinedConstraints
  void discrete_real_upper_bounds(const RealVector& d_u_bnds);

  /// return the inactive continuous lower bounds in userDefinedConstraints
  const RealVector& inactive_continuous_lower_bounds() const;
  /// set the inactive continuous lower bounds in userDefinedConstraints
  void inactive_continuous_lower_bounds(const RealVector& i_c_l_bnds);
  /// return the inactive continuous upper bounds in userDefinedConstraints
  const RealVector& inactive_continuous_upper_bounds() const;
  /// set the inactive continuous upper bounds in userDefinedConstraints
  void inactive_continuous_upper_bounds(const RealVector& i_c_u_bnds);
  /// return the inactive discrete lower bounds in userDefinedConstraints
  const IntVector& inactive_discrete_int_lower_bounds() const;
  /// set the inactive discrete lower bounds in userDefinedConstraints
  void inactive_discrete_int_lower_bounds(const IntVector& i_d_l_bnds);
  /// return the inactive discrete upper bounds in userDefinedConstraints
  const IntVector& inactive_discrete_int_upper_bounds() const;
  /// set the inactive discrete upper bounds in userDefinedConstraints
  void inactive_discrete_int_upper_bounds(const IntVector& i_d_u_bnds);
  /// return the inactive discrete lower bounds in userDefinedConstraints
  const RealVector& inactive_discrete_real_lower_bounds() const;
  /// set the inactive discrete lower bounds in userDefinedConstraints
  void inactive_discrete_real_lower_bounds(const RealVector& i_d_l_bnds);
  /// return the inactive discrete upper bounds in userDefinedConstraints
  const RealVector& inactive_discrete_real_upper_bounds() const;
  /// set the inactive discrete upper bounds in userDefinedConstraints
  void inactive_discrete_real_upper_bounds(const RealVector& i_d_u_bnds);

  /// return all continuous lower bounds in userDefinedConstraints
  const RealVector& all_continuous_lower_bounds() const;
  /// set all continuous lower bounds in userDefinedConstraints
  void all_continuous_lower_bounds(const RealVector& a_c_l_bnds);
  /// set a lower bound within continuous lower bounds in
  /// userDefinedConstraints
  void all_continuous_lower_bound(const Real& a_c_l_bnd, const size_t& i);
  /// return all continuous upper bounds in userDefinedConstraints
  const RealVector& all_continuous_upper_bounds() const;
  /// set all continuous upper bounds in userDefinedConstraints
  void all_continuous_upper_bounds(const RealVector& a_c_u_bnds);
  /// set an upper bound within all continuous upper bounds in
  /// userDefinedConstraints
  void all_continuous_upper_bound(const Real& a_c_u_bnd, const size_t& i);
  /// return all discrete lower bounds in userDefinedConstraints
  const IntVector& all_discrete_int_lower_bounds() const;
  /// set all discrete lower bounds in userDefinedConstraints
  void all_discrete_int_lower_bounds(const IntVector& a_d_l_bnds);
  /// set a lower bound within all discrete lower bounds in
  /// userDefinedConstraints
  void all_discrete_int_lower_bound(const int& a_d_l_bnd, const size_t& i);
  /// return all discrete upper bounds in userDefinedConstraints
  const IntVector& all_discrete_int_upper_bounds() const;
  /// set all discrete upper bounds in userDefinedConstraints
  void all_discrete_int_upper_bounds(const IntVector& a_d_u_bnds);
  /// set an upper bound within all discrete upper bounds in
  /// userDefinedConstraints
  void all_discrete_int_upper_bound(const int& a_d_u_bnd, const size_t& i);
  /// return all discrete lower bounds in userDefinedConstraints
  const RealVector& all_discrete_real_lower_bounds() const;
  /// set all discrete lower bounds in userDefinedConstraints
  void all_discrete_real_lower_bounds(const RealVector& a_d_l_bnds);
  /// set a lower bound within all discrete lower bounds in
  /// userDefinedConstraints
  void all_discrete_real_lower_bound(const Real& a_d_l_bnd, const size_t& i);
  /// return all discrete upper bounds in userDefinedConstraints
  const RealVector& all_discrete_real_upper_bounds() const;
  /// set all discrete upper bounds in userDefinedConstraints
  void all_discrete_real_upper_bounds(const RealVector& a_d_u_bnds);
  /// set an upper bound within all discrete upper bounds in
  /// userDefinedConstraints
  void all_discrete_real_upper_bound(const Real& a_d_u_bnd, const size_t& i);

  // LINEAR CONSTRAINTS

  /// return the number of linear inequality constraints
  size_t num_linear_ineq_constraints() const;
  /// return the number of linear equality constraints
  size_t num_linear_eq_constraints() const;

  /// return the linear inequality constraint coefficients
  const RealMatrix& linear_ineq_constraint_coeffs() const;
  /// set the linear inequality constraint coefficients
  void linear_ineq_constraint_coeffs(const RealMatrix& lin_ineq_coeffs);
  /// return the linear inequality constraint lower bounds
  const RealVector& linear_ineq_constraint_lower_bounds() const;
  /// set the linear inequality constraint lower bounds
  void linear_ineq_constraint_lower_bounds(
    const RealVector& lin_ineq_l_bnds);
  /// return the linear inequality constraint upper bounds
  const RealVector& linear_ineq_constraint_upper_bounds() const;
  /// set the linear inequality constraint upper bounds
  void linear_ineq_constraint_upper_bounds(
    const RealVector& lin_ineq_u_bnds);
  /// return the linear equality constraint coefficients
  const RealMatrix& linear_eq_constraint_coeffs() const;
  /// set the linear equality constraint coefficients
  void linear_eq_constraint_coeffs(const RealMatrix& lin_eq_coeffs);
  /// return the linear equality constraint targets
  const RealVector& linear_eq_constraint_targets() const;
  /// set the linear equality constraint targets
  void linear_eq_constraint_targets(const RealVector& lin_eq_targets);

  // NONLINEAR CONSTRAINTS

  /// return the number of nonlinear inequality constraints
  size_t num_nonlinear_ineq_constraints() const;
  /// return the number of nonlinear equality constraints
  size_t num_nonlinear_eq_constraints() const;

  /// return the nonlinear inequality constraint lower bounds
  const RealVector& nonlinear_ineq_constraint_lower_bounds() const;
  /// set the nonlinear inequality constraint lower bounds
  void nonlinear_ineq_constraint_lower_bounds(
    const RealVector& nln_ineq_l_bnds);
  /// return the nonlinear inequality constraint upper bounds
  const RealVector& nonlinear_ineq_constraint_upper_bounds() const;
  /// set the nonlinear inequality constraint upper bounds
  void nonlinear_ineq_constraint_upper_bounds(
    const RealVector& nln_ineq_u_bnds);
  /// return the nonlinear equality constraint targets
  const RealVector& nonlinear_eq_constraint_targets() const;
  /// set the nonlinear equality constraint targets
  void nonlinear_eq_constraint_targets(const RealVector& nln_eq_targets);

  // return the array of discrete variable identifiers that were
  // relaxed into a continuous array in currentVariables
  //const SizetArray& relaxed_discrete_ids() const;

  /// return the current variables (currentVariables)
  const Variables& current_variables() const;
  /// return the user-defined constraints (userDefinedConstraints)
  const Constraints& user_defined_constraints() const;
  /// return the current response (currentResponse)
  const Response& current_response() const;
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

  /// return number of functions in currentResponse
  size_t num_functions() const;

  /// return the gradient evaluation type (gradType)
  const String& gradient_type() const;
  /// return the numerical gradient evaluation method source (methodSrc)
  const String& method_source() const;
  /// return the numerical gradient evaluation interval type (intervalType)
  const String& interval_type() const;
  /// option for ignoring bounds when numerically estimating derivatives
  bool ignore_bounds() const;
  /// option for using old 2nd-order scheme when computing finite-diff Hessian
  bool central_hess() const;
  /// return the finite difference gradient step size (fdGradSS)
  const RealVector& fd_gradient_step_size() const;
  /// return the mixed gradient analytic IDs (gradIdAnalytic)
  const IntList& gradient_id_analytic() const;
  /// return the mixed gradient numerical IDs (gradIdNumerical)
  const IntList& gradient_id_numerical() const;
  /// return the Hessian evaluation type (hessType)
  const String& hessian_type() const;
  /// return the Hessian evaluation type (quasiHessType)
  const String& quasi_hessian_type() const;
  /// return gradient-based finite difference Hessian step size (fdHessByGradSS)
  const RealVector& fd_hessian_by_grad_step_size() const;
  /// return function-based finite difference Hessian step size (fdHessByFnSS)
  const RealVector& fd_hessian_by_fn_step_size() const;
  /// return the mixed Hessian analytic IDs (hessIdAnalytic)
  const IntList& hessian_id_analytic() const;
  /// return the mixed Hessian analytic IDs (hessIdNumerical)
  const IntList& hessian_id_numerical() const;
  /// return the mixed Hessian analytic IDs (hessIdQuasi)
  const IntList& hessian_id_quasi() const;

  /// get the relative weightings for multiple objective functions or least
  /// squares terms.  Used by ConcurrentStrategy for Pareto set optimization.
  const RealVector& primary_response_fn_weights() const;

  /// indicates potential usage of estimate_derivatives() based on
  /// gradType/hessType
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
  void parallel_configuration_iterator(const ParConfigLIter& pc_iter);
  /// return modelPCIter
  const ParConfigLIter& parallel_configuration_iterator() const;

  /// set modelAutoGraphicsFlag to activate posting of graphics data within
  /// compute_response/synchronize functions (automatic graphics posting in
  /// the model as opposed to graphics posting at the strategy level).
  void auto_graphics(const bool flag);

  /// function to check modelRep (does this envelope contain a letter)
  bool is_null() const;

  /// returns modelRep for access to derived class member functions
  /// that are not mapped to the top Model level
  Model* model_rep() const;

  /// flags finite-difference step size adjusted by bounds
  bool shortStep;
  /// function returning finite-difference step size (affected by bounds)
  Real FDstep1(const Real& x0_j, const Real& lb_j, const Real& ub_j,
	       Real h_mag);
  /// function returning second central-difference step size (affected
  /// by bounds)
  Real FDstep2(const Real& x0_j, const Real& lb_j, const Real& ub_j, Real h);

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializing the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Model(BaseConstructor, ProblemDescDB& problem_db);

  /// constructor initializing base class for derived model class instances
  /// constructed on the fly
  Model(NoDBBaseConstructor, ParallelLibrary& parallel_lib,
	const SharedVariablesData& svd,	const ActiveSet& set,
	short output_level);

  /// constructor initializing base class for recast model class instances
  /// constructed on the fly
  Model(RecastBaseConstructor, ProblemDescDB& problem_db,
	ParallelLibrary& parallel_lib);

  //
  //- Heading: Virtual functions
  //

  /// portion of compute_response() specific to derived model classes
  virtual void derived_compute_response(const ActiveSet& set);
  /// portion of asynch_compute_response() specific to derived model classes
  virtual void derived_asynch_compute_response(const ActiveSet& set);

  /// portion of synchronize() specific to derived model classes
  virtual const IntResponseMap& derived_synchronize();
  /// portion of synchronize_nowait() specific to derived model classes
  virtual const IntResponseMap& derived_synchronize_nowait();

  /// portion of init_communicators() specific to derived model classes
  virtual void derived_init_communicators(int max_iterator_concurrency,
					  bool recurse_flag = true);
  /// portion of init_serial() specific to derived model classes
  virtual void derived_init_serial();
  /// portion of set_communicators() specific to derived model classes
  virtual void derived_set_communicators(int max_iterator_concurrency,
					 bool recurse_flag = true);
  /// portion of free_communicators() specific to derived model classes
  virtual void derived_free_communicators(int max_iterator_concurrency,
					  bool recurse_flag = true);

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

  /// type of model: single, nested, or surrogate
  String modelType;
  /// type of surrogate model: local_*, multipoint_*, global_*, or hierarchical
  String surrogateType;

  String gradType;           ///< grad type: none,numerical,analytic,mixed
  String methodSrc;          ///< method source: dakota,vendor
  String intervalType;       ///< interval type: forward,central
  bool   ignoreBounds;	     ///< option to ignore bounds when computing
                             ///< finite differences
  bool	 centralHess; ///< option to use old 2nd-order finite diffs for Hessians
  RealVector fdGradSS;       ///< relative step sizes for numerical gradients
  IntList gradIdAnalytic;    ///< analytic id's for mixed gradients
  IntList gradIdNumerical;   ///< numerical id's for mixed gradients
  String hessType;           ///< Hess type: none,numerical,quasi,analytic,mixed
  String quasiHessType;      ///< quasi-Hessian type: bfgs, damped_bfgs, sr1
  RealVector fdHessByGradSS; ///< relative step sizes for numerical Hessians
                             ///< estimated with 1st-order grad differences
  RealVector fdHessByFnSS;   ///< relative step sizes for numerical Hessians
                             ///< estimated with 2nd-order fn differences
  IntList hessIdAnalytic;    ///< analytic id's for mixed Hessians
  IntList hessIdNumerical;   ///< numerical id's for mixed Hessians
  IntList hessIdQuasi;       ///< quasi id's for mixed Hessians
  bool supportsEstimDerivs;  ///< whether model should perform or forward
                             ///< derivative estimation

  /// length of packed MPI buffers containing vars, vars/set, response,
  /// and PRPair
  IntArray messageLengths;

  /// class member reference to the problem description database
  ProblemDescDB& probDescDB;

  /// class member reference to the parallel library
  ParallelLibrary& parallelLib;

  /// the ParallelConfiguration node used by this model instance
  ParConfigLIter modelPCIter;

  /// the component parallelism mode: 0 (none), 1 (INTERFACE/LF_MODEL), or 2
  /// (SUB_MODEL/HF_MODEL/TRUTH_MODEL)
  short componentParallelMode;

  /// flags asynch evaluations (local or distributed)
  bool asynchEvalFlag;

  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;

  /// array of IntSet's, each containing the set of allowable integer
  /// values corresponding to discrete design integer set variable
  IntSetArray discreteDesignSetIntValues;
  /// array of RealSet's, each containing the set of allowable real
  /// values corresponding to discrete design real set variable
  RealSetArray discreteDesignSetRealValues;

  /// container for random variable distribution parameters
  Pecos::DistributionParams distParams;

  /// array of IntSet's, each containing the set of allowable integer
  /// values corresponding to discrete state integer set variable
  IntSetArray discreteStateSetIntValues;
  /// array of RealSet's, each containing the set of allowable real
  /// values corresponding to discrete state real set variable
  RealSetArray discreteStateSetRealValues;

  /// primary response function weightings (either weights for
  /// multiobjective optimization or weighted least squares)
  RealVector primaryRespFnWts;

private:
 
  //
  //- Heading: Member functions
  //

  /// Used by the envelope to instantiate the correct letter class
  Model* get_model(ProblemDescDB& problem_db);

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

  /// return the lower bound for a finite difference offset, drawn from
  /// global or distribution bounds
  Real finite_difference_lower_bound(UShortMultiArrayConstView cv_types,
				     const RealVector& global_c_l_bnds,
				     size_t cv_index);
  /// return the upper bound for a finite difference offset, drawn from
  /// global or distribution bounds
  Real finite_difference_upper_bound(UShortMultiArrayConstView cv_types,
				     const RealVector& global_c_u_bnds,
				     size_t cv_index);

  /// Coordinates usage of estimate_derivatives() calls based on asv_in
  bool manage_asv(const ShortArray& asv_in, ShortArray& map_asv_out, 
		  ShortArray& fd_grad_asv_out, ShortArray& fd_hess_asv_out,
		  ShortArray& quasi_hess_asv_out);

  //
  //- Heading: Data
  //

  /// model identifier string from the input file
  String modelId;

  /// evaluation counter for top-level compute_response() and
  /// asynch_compute_response() calls.  Differs from lower level counters
  /// in case of numerical derivative estimation (several lower level
  /// evaluations are assimilated into a single higher level evaluation)
  int modelEvalCntr;

  /// flags presence of estimated derivatives within a set of calls to
  /// asynch_compute_response()
  bool estDerivsFlag;

  /// capacity for concurrent evaluations supported by the Model
  int evaluationCapacity;

  /// map<> used for tracking modelPCIter instances using concurrency
  /// level as the lookup key
  std::map<int, ParConfigLIter> modelPCIterMap;

  /// flag for determining need to bcast the max concurrency from
  /// init_communicators(); set from Strategy::init_iterator()
  bool initCommsBcastFlag;

  /// flag for posting of graphics data within compute_response
  /// (automatic graphics posting in the model as opposed to
  /// graphics posting at the strategy level)
  bool modelAutoGraphicsFlag;

  /// used to collect sub-models for subordinate_models()
  ModelList modelList;

  /// history of vars populated in asynch_compute_response() and used in
  /// synchronize().
  VariablesList varsList;

  /// if estimate_derivatives() is used, transfers ASVs from
  /// asynch_compute_response() to synchronize()
  std::list<ShortArray> asvList;

  /// if estimate_derivatives() is used, transfers ActiveSets from
  /// asynch_compute_response() to synchronize()
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

  /// used to cache the data returned from derived_synchronize_nowait() prior
  /// to sequential input into the graphics
  IntResponseMap graphicsRespMap;

  /// pointer to the letter (initialized only for the envelope)
  Model* modelRep;
  /// number of objects sharing modelRep
  int referenceCount;
};


inline size_t Model::tv() const
{ return (modelRep) ? modelRep->currentVariables.tv() : currentVariables.tv(); }


inline size_t Model::cv() const
{ return (modelRep) ? modelRep->currentVariables.cv() : currentVariables.cv(); }


inline size_t Model::div() const
{
  return (modelRep) ? modelRep->currentVariables.div() : currentVariables.div();
}


inline size_t Model::drv() const
{
  return (modelRep) ? modelRep->currentVariables.drv() : currentVariables.drv();
}


inline size_t Model::icv() const
{
  return (modelRep) ? modelRep->currentVariables.icv() : currentVariables.icv();
}


inline size_t Model::idiv() const
{
  return (modelRep) ? modelRep->currentVariables.idiv()
                    : currentVariables.idiv();
}


inline size_t Model::idrv() const
{
  return (modelRep) ? modelRep->currentVariables.idrv()
                    : currentVariables.idrv();
}


inline size_t Model::acv() const
{
  return (modelRep) ? modelRep->currentVariables.acv() : currentVariables.acv();
}


inline size_t Model::adiv() const
{
  return (modelRep) ? modelRep->currentVariables.adiv()
                    : currentVariables.adiv();
}


inline size_t Model::adrv() const
{
  return (modelRep) ? modelRep->currentVariables.adrv()
                    : currentVariables.adrv();
}


inline void Model::active_variables(const Variables& vars) 
{
  // Set active variables only.  Leave remainder of currentVariables unchanged
  // (so that inactive vars can vary between iterators/models w/i a strategy).
  if (modelRep) // envelope fwd to letter
    modelRep->active_variables(vars);
  else { // letter
    if (vars.cv())
      currentVariables.continuous_variables(vars.continuous_variables());
    if (vars.div())
      currentVariables.discrete_int_variables(vars.discrete_int_variables());
    if (vars.drv())
      currentVariables.discrete_real_variables(vars.discrete_real_variables());
  }
}


inline const RealVector& Model::continuous_variables() const
{
  return (modelRep) ? modelRep->currentVariables.continuous_variables()
                    : currentVariables.continuous_variables();
}


inline void Model::continuous_variables(const RealVector& c_vars) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.continuous_variables(c_vars);
  else // letter
    currentVariables.continuous_variables(c_vars);
}


inline void Model::continuous_variable(const Real& c_var, const size_t& i) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.continuous_variable(c_var, i);
  else // letter
    currentVariables.continuous_variable(c_var, i);
}


inline const IntVector& Model::discrete_int_variables() const
{
  return (modelRep) ? modelRep->currentVariables.discrete_int_variables()
                    : currentVariables.discrete_int_variables();
}


inline void Model::discrete_int_variables(const IntVector& d_vars) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.discrete_int_variables(d_vars);
  else // letter
    currentVariables.discrete_int_variables(d_vars);
}


inline void Model::discrete_int_variable(const int& d_var, const size_t& i) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.discrete_int_variable(d_var, i);
  else // letter
    currentVariables.discrete_int_variable(d_var, i);
}


inline const RealVector& Model::discrete_real_variables() const
{
  return (modelRep) ? modelRep->currentVariables.discrete_real_variables()
                    : currentVariables.discrete_real_variables();
}


inline void Model::discrete_real_variables(const RealVector& d_vars) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.discrete_real_variables(d_vars);
  else // letter
    currentVariables.discrete_real_variables(d_vars);
}


inline void Model::discrete_real_variable(const Real& d_var, const size_t& i) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.discrete_real_variable(d_var, i);
  else // letter
    currentVariables.discrete_real_variable(d_var, i);
}


inline UShortMultiArrayConstView Model::continuous_variable_types() const
{
  return (modelRep) ? modelRep->currentVariables.continuous_variable_types()
                    : currentVariables.continuous_variable_types();
}


inline UShortMultiArrayConstView Model::discrete_int_variable_types() const
{
  return (modelRep) ? modelRep->currentVariables.discrete_int_variable_types()
                    : currentVariables.discrete_int_variable_types();
}


inline UShortMultiArrayConstView Model::discrete_real_variable_types() const
{
  return (modelRep) ? modelRep->currentVariables.discrete_real_variable_types()
                    : currentVariables.discrete_real_variable_types();
}


inline SizetMultiArrayConstView Model::continuous_variable_ids() const
{
  return (modelRep) ? modelRep->currentVariables.continuous_variable_ids()
                    : currentVariables.continuous_variable_ids();
}


inline const RealVector& Model::inactive_continuous_variables() const
{
  return (modelRep) ? modelRep->currentVariables.inactive_continuous_variables()
                    : currentVariables.inactive_continuous_variables();
}


inline void Model::
inactive_continuous_variables(const RealVector& i_c_vars)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.inactive_continuous_variables(i_c_vars);
  else // letter
    currentVariables.inactive_continuous_variables(i_c_vars);
}


inline const IntVector& Model::inactive_discrete_int_variables() const
{
  return (modelRep) ?
    modelRep->currentVariables.inactive_discrete_int_variables() :
    currentVariables.inactive_discrete_int_variables();
}


inline void Model::inactive_discrete_int_variables(const IntVector& i_d_vars)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.inactive_discrete_int_variables(i_d_vars);
  else // letter
    currentVariables.inactive_discrete_int_variables(i_d_vars);
}


inline const RealVector& Model::inactive_discrete_real_variables() const
{
  return (modelRep) ?
    modelRep->currentVariables.inactive_discrete_real_variables() :
    currentVariables.inactive_discrete_real_variables();
}


inline void Model::inactive_discrete_real_variables(const RealVector& i_d_vars)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.inactive_discrete_real_variables(i_d_vars);
  else // letter
    currentVariables.inactive_discrete_real_variables(i_d_vars);
}


inline UShortMultiArrayConstView Model::
inactive_continuous_variable_types() const
{
  return (modelRep) ?
    modelRep->currentVariables.inactive_continuous_variable_types() :
    currentVariables.inactive_continuous_variable_types();
}


inline SizetMultiArrayConstView Model::inactive_continuous_variable_ids() const
{
  return (modelRep) ?
    modelRep->currentVariables.inactive_continuous_variable_ids() :
    currentVariables.inactive_continuous_variable_ids();
}


inline const RealVector& Model::all_continuous_variables() const
{
  return (modelRep) ? modelRep->currentVariables.all_continuous_variables()
                    : currentVariables.all_continuous_variables();
}


inline void Model::all_continuous_variables(const RealVector& a_c_vars)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_continuous_variables(a_c_vars);
  else // letter
    currentVariables.all_continuous_variables(a_c_vars);
}


inline void Model::all_continuous_variable(const Real& a_c_var, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_continuous_variable(a_c_var, i);
  else // letter
    currentVariables.all_continuous_variable(a_c_var, i);
}


inline const IntVector& Model::all_discrete_int_variables() const
{
  return (modelRep) ? modelRep->currentVariables.all_discrete_int_variables()
                    : currentVariables.all_discrete_int_variables();
}


inline void Model::all_discrete_int_variables(const IntVector& a_d_vars)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_int_variables(a_d_vars);
  else // letter
    currentVariables.all_discrete_int_variables(a_d_vars);
}


inline void Model::
all_discrete_int_variable(const int& a_d_var, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_int_variable(a_d_var, i);
  else // letter
    currentVariables.all_discrete_int_variable(a_d_var, i);
}


inline const RealVector& Model::all_discrete_real_variables() const
{
  return (modelRep) ? modelRep->currentVariables.all_discrete_real_variables()
                    : currentVariables.all_discrete_real_variables();
}


inline void Model::all_discrete_real_variables(const RealVector& a_d_vars)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_real_variables(a_d_vars);
  else // letter
    currentVariables.all_discrete_real_variables(a_d_vars);
}


inline void Model::
all_discrete_real_variable(const Real& a_d_var, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_real_variable(a_d_var, i);
  else // letter
    currentVariables.all_discrete_real_variable(a_d_var, i);
}


inline UShortMultiArrayConstView Model::all_continuous_variable_types() const
{
  return (modelRep) ? modelRep->currentVariables.all_continuous_variable_types()
                    : currentVariables.all_continuous_variable_types();
}


inline UShortMultiArrayConstView Model::all_discrete_int_variable_types() const
{
  return (modelRep) ?
    modelRep->currentVariables.all_discrete_int_variable_types() :
    currentVariables.all_discrete_int_variable_types();
}


inline UShortMultiArrayConstView Model::all_discrete_real_variable_types() const
{
  return (modelRep) ?
    modelRep->currentVariables.all_discrete_real_variable_types() :
    currentVariables.all_discrete_real_variable_types();
}


inline SizetMultiArrayConstView Model::all_continuous_variable_ids() const
{
  return (modelRep) ? modelRep->currentVariables.all_continuous_variable_ids()
                    : currentVariables.all_continuous_variable_ids();
}


inline const IntSetArray& Model::discrete_design_set_int_values() const
{
  return (modelRep) ? modelRep->discreteDesignSetIntValues
                    : discreteDesignSetIntValues;
}


inline void Model::discrete_design_set_int_values(const IntSetArray& isa)
{
  if (modelRep)
    modelRep->discreteDesignSetIntValues = isa;
  else
    discreteDesignSetIntValues = isa;
}


inline const RealSetArray& Model::discrete_design_set_real_values() const
{
  return (modelRep) ? modelRep->discreteDesignSetRealValues
                    : discreteDesignSetRealValues;
}


inline void Model::discrete_design_set_real_values(const RealSetArray& rsa)
{
  if (modelRep)
    modelRep->discreteDesignSetRealValues = rsa;
  else
    discreteDesignSetRealValues = rsa;
}


inline Pecos::DistributionParams& Model::distribution_parameters()
{ return (modelRep) ? modelRep->distParams : distParams; }


inline const Pecos::DistributionParams& Model::distribution_parameters() const
{ return (modelRep) ? modelRep->distParams : distParams; }


inline void Model::distribution_parameters(const Pecos::DistributionParams& dp)
{
  if (modelRep)
    modelRep->distParams = dp;
  else
    distParams = dp;
}


inline const IntSetArray& Model::discrete_state_set_int_values() const
{
  return (modelRep) ? modelRep->discreteStateSetIntValues
                    : discreteStateSetIntValues;
}


inline void Model::discrete_state_set_int_values(const IntSetArray& isa)
{
  if (modelRep)
    modelRep->discreteStateSetIntValues = isa;
  else
    discreteStateSetIntValues = isa;
}


inline const RealSetArray& Model::discrete_state_set_real_values() const
{
  return (modelRep) ? modelRep->discreteStateSetRealValues
                    : discreteStateSetRealValues;
}


inline void Model::discrete_state_set_real_values(const RealSetArray& rsa)
{
  if (modelRep)
    modelRep->discreteStateSetRealValues = rsa;
  else
    discreteStateSetRealValues = rsa;
}


inline StringMultiArrayConstView Model::continuous_variable_labels() const
{
  return (modelRep) ? modelRep->currentVariables.continuous_variable_labels()
                    : currentVariables.continuous_variable_labels();
}


inline void Model::
continuous_variable_labels(StringMultiArrayConstView c_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.continuous_variable_labels(c_v_labels);
  else // letter
    currentVariables.continuous_variable_labels(c_v_labels);
}


inline StringMultiArrayConstView Model::discrete_int_variable_labels() const
{
  return (modelRep) ? modelRep->currentVariables.discrete_int_variable_labels()
                    : currentVariables.discrete_int_variable_labels();
}


inline void Model::
discrete_int_variable_labels(StringMultiArrayConstView d_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.discrete_int_variable_labels(d_v_labels);
  else // letter
    currentVariables.discrete_int_variable_labels(d_v_labels);
}


inline StringMultiArrayConstView Model::discrete_real_variable_labels() const
{
  return (modelRep) ? modelRep->currentVariables.discrete_real_variable_labels()
                    : currentVariables.discrete_real_variable_labels();
}


inline void Model::
discrete_real_variable_labels(StringMultiArrayConstView d_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.discrete_real_variable_labels(d_v_labels);
  else // letter
    currentVariables.discrete_real_variable_labels(d_v_labels);
}


inline StringMultiArrayConstView Model::
inactive_continuous_variable_labels() const
{
  return (modelRep) ? 
    modelRep->currentVariables.inactive_continuous_variable_labels() :
    currentVariables.inactive_continuous_variable_labels();
}


inline void Model::
inactive_continuous_variable_labels(StringMultiArrayConstView i_c_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      currentVariables.inactive_continuous_variable_labels(i_c_v_labels);
  else // letter
    currentVariables.inactive_continuous_variable_labels(i_c_v_labels);
}


inline StringMultiArrayConstView Model::
inactive_discrete_int_variable_labels() const
{
  return (modelRep) ? 
    modelRep->currentVariables.inactive_discrete_int_variable_labels() :
    currentVariables.inactive_discrete_int_variable_labels();
}


inline void Model::
inactive_discrete_int_variable_labels(StringMultiArrayConstView i_d_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      currentVariables.inactive_discrete_int_variable_labels(i_d_v_labels);
  else // letter
    currentVariables.inactive_discrete_int_variable_labels(i_d_v_labels);
}


inline StringMultiArrayConstView Model::
inactive_discrete_real_variable_labels() const
{
  return (modelRep) ? 
    modelRep->currentVariables.inactive_discrete_real_variable_labels() :
    currentVariables.inactive_discrete_real_variable_labels();
}


inline void Model::
inactive_discrete_real_variable_labels(StringMultiArrayConstView i_d_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      currentVariables.inactive_discrete_real_variable_labels(i_d_v_labels);
  else // letter
    currentVariables.inactive_discrete_real_variable_labels(i_d_v_labels);
}


inline StringMultiArrayConstView Model::all_continuous_variable_labels() const
{
  return (modelRep) ? 
    modelRep->currentVariables.all_continuous_variable_labels() :
    currentVariables.all_continuous_variable_labels();
}


inline void Model::
all_continuous_variable_labels(StringMultiArrayConstView a_c_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_continuous_variable_labels(a_c_v_labels);
  else // letter
    currentVariables.all_continuous_variable_labels(a_c_v_labels);
}


inline void Model::
all_continuous_variable_label(const String& a_c_v_label, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_continuous_variable_label(a_c_v_label, i);
  else // letter
    currentVariables.all_continuous_variable_label(a_c_v_label, i);
}


inline StringMultiArrayConstView Model::all_discrete_int_variable_labels() const
{
  return (modelRep) ? 
    modelRep->currentVariables.all_discrete_int_variable_labels() :
    currentVariables.all_discrete_int_variable_labels();
}


inline void Model::
all_discrete_int_variable_labels(StringMultiArrayConstView a_d_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_int_variable_labels(a_d_v_labels);
  else // letter
    currentVariables.all_discrete_int_variable_labels(a_d_v_labels);
}


inline void Model::
all_discrete_int_variable_label(const String& a_d_v_label, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_int_variable_label(a_d_v_label, i);
  else // letter
    currentVariables.all_discrete_int_variable_label(a_d_v_label, i);
}


inline StringMultiArrayConstView Model::
all_discrete_real_variable_labels() const
{
  return (modelRep) ? 
    modelRep->currentVariables.all_discrete_real_variable_labels() :
    currentVariables.all_discrete_real_variable_labels();
}


inline void Model::
all_discrete_real_variable_labels(StringMultiArrayConstView a_d_v_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_real_variable_labels(a_d_v_labels);
  else // letter
    currentVariables.all_discrete_real_variable_labels(a_d_v_labels);
}


inline void Model::
all_discrete_real_variable_label(const String& a_d_v_label, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentVariables.all_discrete_real_variable_label(a_d_v_label, i);
  else // letter
    currentVariables.all_discrete_real_variable_label(a_d_v_label, i);
}


inline const StringArray& Model::response_labels() const
{
  return (modelRep) ? modelRep->currentResponse.function_labels()
                    : currentResponse.function_labels();
}


inline void Model::response_labels(const StringArray& resp_labels)
{
  if (modelRep) // envelope fwd to letter
    modelRep->currentResponse.function_labels(resp_labels);
  else // letter
    currentResponse.function_labels(resp_labels);
}


inline const RealVector& Model::continuous_lower_bounds() const
{
  return (modelRep) ? modelRep->userDefinedConstraints.continuous_lower_bounds()
                    : userDefinedConstraints.continuous_lower_bounds();
}


inline void Model::continuous_lower_bounds(const RealVector& c_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.continuous_lower_bounds(c_l_bnds);
  else // letter
    userDefinedConstraints.continuous_lower_bounds(c_l_bnds);
}


inline const RealVector& Model::continuous_upper_bounds() const
{
  return (modelRep) ? modelRep->userDefinedConstraints.continuous_upper_bounds()
                    : userDefinedConstraints.continuous_upper_bounds();
}


inline void Model::continuous_upper_bounds(const RealVector& c_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.continuous_upper_bounds(c_u_bnds);
  else // letter
    userDefinedConstraints.continuous_upper_bounds(c_u_bnds);
}


inline const IntVector& Model::discrete_int_lower_bounds() const
{
  return (modelRep) ?
    modelRep->userDefinedConstraints.discrete_int_lower_bounds() :
    userDefinedConstraints.discrete_int_lower_bounds();
}


inline void Model::discrete_int_lower_bounds(const IntVector& d_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.discrete_int_lower_bounds(d_l_bnds);
  else // letter
    userDefinedConstraints.discrete_int_lower_bounds(d_l_bnds);
}


inline const IntVector& Model::discrete_int_upper_bounds() const
{
  return (modelRep) ?
    modelRep->userDefinedConstraints.discrete_int_upper_bounds() :
    userDefinedConstraints.discrete_int_upper_bounds();
}


inline void Model::discrete_int_upper_bounds(const IntVector& d_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.discrete_int_upper_bounds(d_u_bnds);
  else // letter
    userDefinedConstraints.discrete_int_upper_bounds(d_u_bnds);
}


inline const RealVector& Model::discrete_real_lower_bounds() const
{
  return (modelRep) ?
    modelRep->userDefinedConstraints.discrete_real_lower_bounds() :
    userDefinedConstraints.discrete_real_lower_bounds();
}


inline void Model::discrete_real_lower_bounds(const RealVector& d_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.discrete_real_lower_bounds(d_l_bnds);
  else // letter
    userDefinedConstraints.discrete_real_lower_bounds(d_l_bnds);
}


inline const RealVector& Model::discrete_real_upper_bounds() const
{
  return (modelRep) ?
    modelRep->userDefinedConstraints.discrete_real_upper_bounds() :
    userDefinedConstraints.discrete_real_upper_bounds();
}


inline void Model::discrete_real_upper_bounds(const RealVector& d_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.discrete_real_upper_bounds(d_u_bnds);
  else // letter
    userDefinedConstraints.discrete_real_upper_bounds(d_u_bnds);
}


inline const RealVector& Model::inactive_continuous_lower_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.inactive_continuous_lower_bounds() :
    userDefinedConstraints.inactive_continuous_lower_bounds();
}


inline void Model::
inactive_continuous_lower_bounds(const RealVector& i_c_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.inactive_continuous_lower_bounds(i_c_l_bnds);
  else // letter
    userDefinedConstraints.inactive_continuous_lower_bounds(i_c_l_bnds);
}


inline const RealVector& Model::inactive_continuous_upper_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.inactive_continuous_upper_bounds() :
    userDefinedConstraints.inactive_continuous_upper_bounds();
}


inline void Model::
inactive_continuous_upper_bounds(const RealVector& i_c_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.inactive_continuous_upper_bounds(i_c_u_bnds);
  else // letter
    userDefinedConstraints.inactive_continuous_upper_bounds(i_c_u_bnds);
}


inline const IntVector& Model::inactive_discrete_int_lower_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.inactive_discrete_int_lower_bounds() :
    userDefinedConstraints.inactive_discrete_int_lower_bounds();
}


inline void Model::
inactive_discrete_int_lower_bounds(const IntVector& i_d_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.inactive_discrete_int_lower_bounds(i_d_l_bnds);
  else // letter
    userDefinedConstraints.inactive_discrete_int_lower_bounds(i_d_l_bnds);
}


inline const IntVector& Model::inactive_discrete_int_upper_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.inactive_discrete_int_upper_bounds() :
    userDefinedConstraints.inactive_discrete_int_upper_bounds();
}


inline void Model::
inactive_discrete_int_upper_bounds(const IntVector& i_d_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.inactive_discrete_int_upper_bounds(i_d_u_bnds);
  else // letter
    userDefinedConstraints.inactive_discrete_int_upper_bounds(i_d_u_bnds);
}


inline const RealVector& Model::inactive_discrete_real_lower_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.inactive_discrete_real_lower_bounds() :
    userDefinedConstraints.inactive_discrete_real_lower_bounds();
}


inline void Model::
inactive_discrete_real_lower_bounds(const RealVector& i_d_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.inactive_discrete_real_lower_bounds(i_d_l_bnds);
  else // letter
    userDefinedConstraints.inactive_discrete_real_lower_bounds(i_d_l_bnds);
}


inline const RealVector& Model::inactive_discrete_real_upper_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.inactive_discrete_real_upper_bounds() :
    userDefinedConstraints.inactive_discrete_real_upper_bounds();
}


inline void Model::
inactive_discrete_real_upper_bounds(const RealVector& i_d_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.inactive_discrete_real_upper_bounds(i_d_u_bnds);
  else // letter
    userDefinedConstraints.inactive_discrete_real_upper_bounds(i_d_u_bnds);
}


inline const RealVector& Model::all_continuous_lower_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.all_continuous_lower_bounds() :
    userDefinedConstraints.all_continuous_lower_bounds();
}


inline void Model::
all_continuous_lower_bounds(const RealVector& a_c_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_continuous_lower_bounds(a_c_l_bnds);
  else // letter
    userDefinedConstraints.all_continuous_lower_bounds(a_c_l_bnds);
}


inline void Model::
all_continuous_lower_bound(const Real& a_c_l_bnd, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_continuous_lower_bound(a_c_l_bnd, i);
  else // letter
    userDefinedConstraints.all_continuous_lower_bound(a_c_l_bnd, i);
}


inline const RealVector& Model::all_continuous_upper_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.all_continuous_upper_bounds() :
    userDefinedConstraints.all_continuous_upper_bounds();
}


inline void Model::
all_continuous_upper_bounds(const RealVector& a_c_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_continuous_upper_bounds(a_c_u_bnds);
  else // letter
    userDefinedConstraints.all_continuous_upper_bounds(a_c_u_bnds);
}


inline void Model::
all_continuous_upper_bound(const Real& a_c_u_bnd, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_continuous_upper_bound(a_c_u_bnd, i);
  else // letter
    userDefinedConstraints.all_continuous_upper_bound(a_c_u_bnd, i);
}


inline const IntVector& Model::all_discrete_int_lower_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.all_discrete_int_lower_bounds() :
    userDefinedConstraints.all_discrete_int_lower_bounds();
}


inline void Model::all_discrete_int_lower_bounds(const IntVector& a_d_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_discrete_int_lower_bounds(a_d_l_bnds);
  else // letter
    userDefinedConstraints.all_discrete_int_lower_bounds(a_d_l_bnds);
}


inline void Model::
all_discrete_int_lower_bound(const int& a_d_l_bnd, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_discrete_int_lower_bound(a_d_l_bnd, i);
  else // letter
    userDefinedConstraints.all_discrete_int_lower_bound(a_d_l_bnd, i);
}


inline const IntVector& Model::all_discrete_int_upper_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.all_discrete_int_upper_bounds() :
    userDefinedConstraints.all_discrete_int_upper_bounds();
}


inline void Model::all_discrete_int_upper_bounds(const IntVector& a_d_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_discrete_int_upper_bounds(a_d_u_bnds);
  else // letter
    userDefinedConstraints.all_discrete_int_upper_bounds(a_d_u_bnds);
}


inline void Model::
all_discrete_int_upper_bound(const int& a_d_u_bnd, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_discrete_int_upper_bound(a_d_u_bnd, i);
  else // letter
    userDefinedConstraints.all_discrete_int_upper_bound(a_d_u_bnd, i);
}


inline const RealVector& Model::all_discrete_real_lower_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.all_discrete_real_lower_bounds() :
    userDefinedConstraints.all_discrete_real_lower_bounds();
}


inline void Model::all_discrete_real_lower_bounds(const RealVector& a_d_l_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_discrete_real_lower_bounds(a_d_l_bnds);
  else // letter
    userDefinedConstraints.all_discrete_real_lower_bounds(a_d_l_bnds);
}


inline void Model::
all_discrete_real_lower_bound(const Real& a_d_l_bnd, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.all_discrete_real_lower_bound(a_d_l_bnd, i);
  else // letter
    userDefinedConstraints.all_discrete_real_lower_bound(a_d_l_bnd, i);
}


inline const RealVector& Model::all_discrete_real_upper_bounds() const
{
  return (modelRep) ? 
    modelRep->userDefinedConstraints.all_discrete_real_upper_bounds() :
    userDefinedConstraints.all_discrete_real_upper_bounds();
}


inline void Model::all_discrete_real_upper_bounds(const RealVector& a_d_u_bnds)
{
  if (modelRep) // envelope fwd to letter
    modelRep->userDefinedConstraints.all_discrete_real_upper_bounds(a_d_u_bnds);
  else // letter
    userDefinedConstraints.all_discrete_real_upper_bounds(a_d_u_bnds);
}


inline void Model::
all_discrete_real_upper_bound(const Real& a_d_u_bnd, const size_t& i)
{
  if (modelRep) // envelope fwd to letter
    modelRep->
      userDefinedConstraints.all_discrete_real_upper_bound(a_d_u_bnd, i);
  else // letter
    userDefinedConstraints.all_discrete_real_upper_bound(a_d_u_bnd, i);
}


inline size_t Model::num_linear_ineq_constraints() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.num_linear_ineq_constraints()
                    : userDefinedConstraints.num_linear_ineq_constraints();
}


inline size_t Model::num_linear_eq_constraints() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.num_linear_eq_constraints()
                    : userDefinedConstraints.num_linear_eq_constraints();
}


inline const RealMatrix& Model::linear_ineq_constraint_coeffs() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.linear_ineq_constraint_coeffs()
                    : userDefinedConstraints.linear_ineq_constraint_coeffs();
}


inline void Model::
linear_ineq_constraint_coeffs(const RealMatrix& lin_ineq_coeffs)
{
  if (modelRep)
    modelRep->userDefinedConstraints.linear_ineq_constraint_coeffs(
      lin_ineq_coeffs);
  else
    userDefinedConstraints.linear_ineq_constraint_coeffs(lin_ineq_coeffs);
}


inline const RealVector& Model::linear_ineq_constraint_lower_bounds() const
{
  return (modelRep) ?
      modelRep->userDefinedConstraints.linear_ineq_constraint_lower_bounds()
    : userDefinedConstraints.linear_ineq_constraint_lower_bounds();
}


inline void Model::
linear_ineq_constraint_lower_bounds(const RealVector& lin_ineq_l_bnds)
{
  if (modelRep)
    modelRep->userDefinedConstraints.linear_ineq_constraint_lower_bounds(
      lin_ineq_l_bnds);
  else
    userDefinedConstraints.linear_ineq_constraint_lower_bounds(lin_ineq_l_bnds);
}


inline const RealVector& Model::linear_ineq_constraint_upper_bounds() const
{
  return (modelRep) ?
      modelRep->userDefinedConstraints.linear_ineq_constraint_upper_bounds()
    : userDefinedConstraints.linear_ineq_constraint_upper_bounds();
}


inline void Model::
linear_ineq_constraint_upper_bounds(const RealVector& lin_ineq_u_bnds)
{
  if (modelRep)
    modelRep->userDefinedConstraints.linear_ineq_constraint_upper_bounds(
      lin_ineq_u_bnds);
  else
    userDefinedConstraints.linear_ineq_constraint_upper_bounds(lin_ineq_u_bnds);
}


inline const RealMatrix& Model::linear_eq_constraint_coeffs() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.linear_eq_constraint_coeffs()
                    : userDefinedConstraints.linear_eq_constraint_coeffs();
}


inline void Model::
linear_eq_constraint_coeffs(const RealMatrix& lin_eq_coeffs)
{
  if (modelRep)
    modelRep->userDefinedConstraints.linear_eq_constraint_coeffs(lin_eq_coeffs);
  else
    userDefinedConstraints.linear_eq_constraint_coeffs(lin_eq_coeffs);
}


inline const RealVector& Model::linear_eq_constraint_targets() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.linear_eq_constraint_targets()
                    : userDefinedConstraints.linear_eq_constraint_targets();
}


inline void Model::
linear_eq_constraint_targets(const RealVector& lin_eq_targets)
{
  if (modelRep)
    modelRep->userDefinedConstraints.linear_eq_constraint_targets(
      lin_eq_targets);
  else
    userDefinedConstraints.linear_eq_constraint_targets(lin_eq_targets);
}


inline size_t Model::num_nonlinear_ineq_constraints() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.num_nonlinear_ineq_constraints()
                    : userDefinedConstraints.num_nonlinear_ineq_constraints();
}


inline size_t Model::num_nonlinear_eq_constraints() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.num_nonlinear_eq_constraints()
                    : userDefinedConstraints.num_nonlinear_eq_constraints();
}


inline const RealVector& Model::
nonlinear_ineq_constraint_lower_bounds() const
{
  return (modelRep) ?
      modelRep->userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds()
    : userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds();
}


inline void Model::
nonlinear_ineq_constraint_lower_bounds(const RealVector& nln_ineq_l_bnds)
{
  if (modelRep)
    modelRep->userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds(
      nln_ineq_l_bnds);
  else
    userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds(
      nln_ineq_l_bnds);
}


inline const RealVector& Model::
nonlinear_ineq_constraint_upper_bounds() const
{
  return (modelRep) ?
      modelRep->userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds()
    : userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds();
}


inline void Model::
nonlinear_ineq_constraint_upper_bounds(const RealVector& nln_ineq_u_bnds)
{
  if (modelRep)
    modelRep->userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds(
      nln_ineq_u_bnds);
  else
    userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds(
      nln_ineq_u_bnds);
}


inline const RealVector& Model::nonlinear_eq_constraint_targets() const
{
  return (modelRep) ? modelRep->
                      userDefinedConstraints.nonlinear_eq_constraint_targets()
                    : userDefinedConstraints.nonlinear_eq_constraint_targets();
}


inline void Model::
nonlinear_eq_constraint_targets(const RealVector& nln_eq_targets)
{
  if (modelRep)
    modelRep->userDefinedConstraints.nonlinear_eq_constraint_targets(
      nln_eq_targets);
  else
    userDefinedConstraints.nonlinear_eq_constraint_targets(nln_eq_targets);
}


//inline const SizetArray& Model::relaxed_discrete_ids() const
//{
//  return (modelRep) ? modelRep->currentVariables.relaxed_discrete_ids()
//                    : currentVariables.relaxed_discrete_ids();
//}


inline const Variables& Model::current_variables() const
{ return (modelRep) ? modelRep->currentVariables : currentVariables; }


inline const Constraints& Model::user_defined_constraints() const
{
  return (modelRep) ? modelRep->userDefinedConstraints
                    : userDefinedConstraints;
}


inline const Response& Model::current_response() const
{ return (modelRep) ? modelRep->currentResponse : currentResponse; }


inline ProblemDescDB& Model::problem_description_db() const
{ return (modelRep) ? modelRep->probDescDB : probDescDB; }


inline ParallelLibrary& Model::parallel_library() const
{ return (modelRep) ? modelRep->parallelLib : parallelLib; }


inline const String& Model::model_type() const
{ return (modelRep) ? modelRep->modelType : modelType; }


inline const String& Model::surrogate_type() const
{ return (modelRep) ? modelRep->surrogateType : surrogateType; }


inline const String& Model::model_id() const
{ return (modelRep) ? modelRep->modelId : modelId; }


inline size_t Model::num_functions() const
{
  return (modelRep) ? modelRep->currentResponse.num_functions()
                    : currentResponse.num_functions();
}


inline const String& Model::gradient_type() const
{ return (modelRep) ? modelRep->gradType : gradType; }


inline const String& Model::method_source() const
{ return (modelRep) ? modelRep->methodSrc : methodSrc; }


inline const String& Model::interval_type() const
{ return (modelRep) ? modelRep->intervalType : intervalType; }


inline bool Model::ignore_bounds() const
{ return (modelRep) ? modelRep->ignoreBounds : ignoreBounds; }


inline bool Model::central_hess() const
{ return (modelRep) ? modelRep->centralHess : centralHess; }


inline const RealVector& Model::fd_gradient_step_size() const
{ return (modelRep) ? modelRep->fdGradSS : fdGradSS; }


inline const IntList& Model::gradient_id_analytic() const
{ return (modelRep) ? modelRep->gradIdAnalytic : gradIdAnalytic; }


inline const IntList& Model::gradient_id_numerical() const
{ return (modelRep) ? modelRep->gradIdNumerical : gradIdNumerical; }


inline const String& Model::hessian_type() const
{ return (modelRep) ? modelRep->hessType : hessType; }


inline const String& Model::quasi_hessian_type() const
{ return (modelRep) ? modelRep->quasiHessType : quasiHessType; }


inline const RealVector& Model::fd_hessian_by_grad_step_size() const
{ return (modelRep) ? modelRep-> fdHessByGradSS : fdHessByGradSS; }


inline const RealVector& Model::fd_hessian_by_fn_step_size() const
{ return (modelRep) ? modelRep->fdHessByFnSS : fdHessByFnSS; }


inline const IntList& Model::hessian_id_analytic() const
{ return (modelRep) ? modelRep->hessIdAnalytic : hessIdAnalytic; }


inline const IntList& Model::hessian_id_numerical() const
{ return (modelRep) ? modelRep->hessIdNumerical : hessIdNumerical; }


inline const IntList& Model::hessian_id_quasi() const
{ return (modelRep) ? modelRep->hessIdQuasi : hessIdQuasi; }


inline const RealVector& Model::primary_response_fn_weights() const
{ return (modelRep) ? modelRep->primaryRespFnWts : primaryRespFnWts; }


inline bool Model::derivative_estimation()
{
  return (modelRep) ? modelRep->derivative_estimation() :
    ( (gradType == "numerical" || gradType == "mixed") ||
      (hessType == "numerical" || hessType == "mixed" || hessType == "quasi") );
}


inline void Model::supports_derivative_estimation(bool sed_flag)
{
  if (modelRep)
    modelRep->supportsEstimDerivs = sed_flag;
  else
    supportsEstimDerivs = sed_flag;
}


inline void Model::init_comms_bcast_flag(bool icb_flag) 
{
  if (modelRep) // envelope fwd to letter
    modelRep->initCommsBcastFlag = icb_flag;
  else // letter
    initCommsBcastFlag = icb_flag;
}


inline int Model::evaluation_capacity() const
{ return (modelRep) ? modelRep->evaluationCapacity : evaluationCapacity; }


inline bool Model::asynch_flag() const
{ return (modelRep) ? modelRep->asynchEvalFlag : asynchEvalFlag; }


inline void Model::asynch_flag(const bool flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->asynchEvalFlag = flag;
  else // letter
    asynchEvalFlag = flag;
}


inline short Model::output_level() const
{ return (modelRep) ? modelRep->outputLevel : outputLevel; }


inline void Model::output_level(const short level)
{
  if (modelRep) // envelope fwd to letter
    modelRep->outputLevel = level;
  else // letter
    outputLevel = level;
}


inline const IntArray& Model::message_lengths() const
{ return (modelRep) ? modelRep->messageLengths : messageLengths; }


inline void Model::
parallel_configuration_iterator(const ParConfigLIter& pc_iter)
{
  if (modelRep) // envelope fwd to letter
    modelRep->modelPCIter = pc_iter;
  else // letter
    modelPCIter = pc_iter;
}


inline const ParConfigLIter& Model::parallel_configuration_iterator() const
{ return (modelRep) ? modelRep->modelPCIter : modelPCIter; }


inline void Model::auto_graphics(const bool flag)
{
  if (modelRep) // envelope fwd to letter
    modelRep->modelAutoGraphicsFlag = flag;
  else // letter
    modelAutoGraphicsFlag = flag;
}


inline bool Model::is_null() const
{ return (modelRep) ? false : true; }


inline Model* Model::model_rep() const
{ return modelRep; }


/// global comparison function for Model
inline bool model_id_compare(const Model& model, const void* id)
{ return ( *(const String*)id == model.model_id() ); }

} // namespace Dakota

#endif
