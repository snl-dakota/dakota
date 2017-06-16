/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NestedModel
//- Description: A model which invokes a subIterator/subModel for every 
//-              response computation.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: NestedModel.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef NESTED_MODEL_H
#define NESTED_MODEL_H

#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "DakotaIterator.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"
#include "PRPMultiIndex.hpp"
#include "IteratorScheduler.hpp"


namespace Dakota {


/// Derived model class which performs a complete sub-iterator
/// execution within every evaluation of the model.

/** The NestedModel class nests a sub-iterator execution within every
    model evaluation.  This capability is most commonly used for
    optimization under uncertainty, in which a nondeterministic
    iterator is executed on every optimization function evaluation.
    The NestedModel also contains an optional interface, for portions
    of the model evaluation which are independent from the
    sub-iterator, and a set of mappings for combining sub-iterator and
    optional interface data into a top level response for the model. */

class NestedModel: public Model
{
  //
  //- Heading: Friends
  //

  /// protect scheduler callback functions from general access
  friend class IteratorScheduler;

public:
  
  //
  //- Heading: Constructors and destructor
  //

  NestedModel(ProblemDescDB& problem_db); ///< constructor
  ~NestedModel();                         ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // Perform the response computation portions specific to this derived class.
  // In this case, this involves running the subIterator on the subModel.

  /// portion of evaluate() specific to NestedModel
  void derived_evaluate(const ActiveSet& set);
  /// portion of evaluate_nowait() specific to NestedModel
  void derived_evaluate_nowait(const ActiveSet& set);
  /// portion of synchronize() specific to NestedModel
  const IntResponseMap& derived_synchronize();
  // portion of synchronize_nowait() specific to NestedModel
  //const IntResponseMap& derived_synchronize_nowait();

  /// return subIterator
  Iterator& subordinate_iterator();
  /// return subModel
  Model& subordinate_model();
  /// return subModel
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);
  /// return optionalInterface
  Interface& derived_interface();

  /// retrieve error estimates corresponding to the subIterator's response
  /// results (e.g., statistical MSE for subordinate UQ).
  const RealVector& error_estimates();

  /// pass a bypass request on to the subModel for any lower-level surrogates
  void surrogate_response_mode(short mode);

  /// update component parallel mode for supporting parallelism in
  /// optionalInterface and subModel
  void component_parallel_mode(short mode);

  /// return subIteratorSched.miPLIndex
  size_t mi_parallel_level_index() const;

  /// return optionalInterface synchronization setting
  short local_eval_synchronization();
  /// return optionalInterface asynchronous evaluation concurrency
  int local_eval_concurrency();
  /// flag which prevents overloading the master with a multiprocessor
  /// evaluation (forwarded to optionalInterface)
  bool derived_master_overload() const;

  IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// set up optionalInterface and subModel for parallel operations
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);
  /// set up optionalInterface and subModel for serial operations.
  void derived_init_serial();
  /// set active parallel configuration within subModel
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the NestedModel
  /// (forwarded to optionalInterface and subModel)
  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);

  /// Service optionalInterface and subModel job requests received from
  /// the master.  Completes when a termination message is received from
  /// stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);
  /// Executed by the master to terminate server operations for subModel and
  /// optionalInterface when iteration on the NestedModel is complete.
  void stop_servers();

  /// return the optionalInterface identifier
  const String& interface_id() const;
  /// Return the current evaluation id for the NestedModel
  int derived_evaluation_id() const;

  /// set the evaluation counter reference points for the NestedModel
  /// (request forwarded to optionalInterface and subModel)
  void set_evaluation_reference();
  /// request fine-grained evaluation reporting within optionalInterface
  /// and subModel
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the NestedModel
  /// (request forwarded to optionalInterface and subModel)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const;

  /// set the warm start flag, including actualModel
  void warm_start_flag(const bool flag);

  //
  //- Heading: Member functions
  //

  void initialize_iterator(int job_index);
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  void unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer,
				    int job_index);
  void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  void update_local_results(int job_index);

private:

  //
  //- Heading: Convenience member functions
  //

  /// update subIterator with mapping data and set subIterator-based counts
  void update_sub_iterator();

  /// convert job_index to an eval_id through subIteratorIdMap and
  /// eval_id to a subIteratorPRPQueue queue iterator
  PRPQueueIter job_index_to_queue_iterator(int job_index);
  /// lower level function shared by initialize_iterator(int) and
  /// unpack_parameters_initialize()
  void initialize_iterator(const Variables& vars, const ActiveSet& set,
			   int eval_id);
  /// lower level function shared by unpack_parameters_buffer() and
  /// unpack_parameters_initialize()
  void unpack(MPIUnpackBuffer& recv_buffer, int job_index, Variables& vars,
	      ActiveSet& set, int& eval_id);

  /// for a named real mapping, resolve primary index and secondary target
  void resolve_real_variable_mapping(const String& map1, const String& map2,
				     size_t curr_index,
				     short& inactive_sm_view);
  /// for a named integer mapping, resolve primary index and secondary target
  void resolve_integer_variable_mapping(const String& map1, const String& map2,
					size_t curr_index,
					short& inactive_sm_view);
  /// for a named string mapping, resolve primary index and secondary target
  void resolve_string_variable_mapping(const String& map1, const String& map2,
				       size_t curr_index,
				       short& inactive_sm_view);

  /// offset pacvm_index based on sacvm_target to create mapped_index
  size_t sm_acv_index_map(size_t  pacvm_index,  short sacvm_target);
  /// offset padivm_index based on sadivm_target to create mapped_index
  size_t sm_adiv_index_map(size_t padivm_index, short sadivm_target);
  /// offset padsvm_index based on sadsvm_target to create mapped_index
  size_t sm_adsv_index_map(size_t padsvm_index, short sadsvm_target);
  /// offset padrvm_index based on sadrvm_target to create mapped_index
  size_t sm_adrv_index_map(size_t padrvm_index, short sadrvm_target);

  /// offset cv_index to create index into aggregated primary/secondary arrays
  size_t cv_index_map(size_t cv_index, const Variables& vars);
  /// offset div_index to create index into aggregated primary/secondary arrays
  size_t div_index_map(size_t div_index, const Variables& vars);
  /// offset dsv_index to create index into aggregated primary/secondary arrays
  size_t dsv_index_map(size_t dsv_index,
		       const Variables& vars);
  /// offset drv_index to create index into aggregated primary/secondary arrays
  size_t drv_index_map(size_t drv_index, const Variables& vars);

  /// offset active complement ccv_index to create index into all
  /// continuous arrays
  size_t ccv_index_map(size_t ccv_index, const Variables& vars);
  /// offset active complement cdiv_index to create index into all
  /// discrete int arrays
  size_t cdiv_index_map(size_t cdiv_index, const Variables& vars);
  /// offset active complement cdsv_index to create index into all
  /// discrete string arrays
  size_t cdsv_index_map(size_t cdsv_index, const Variables& vars);
  /// offset active complement cdrv_index to create index into all
  /// discrete real arrays
  size_t cdrv_index_map(size_t cdrv_index, const Variables& vars);

  /// insert r_var into appropriate recipient
  void real_variable_mapping(const Real& r_var, size_t mapped_index,
			     short svm_target);
  /// insert i_var into appropriate recipient
  void integer_variable_mapping(const int& i_var, size_t mapped_index,
				short svm_target);
  /// insert s_var into appropriate recipient
  void string_variable_mapping(const String& s_var, size_t mapped_index,
			       short svm_target);

  /// define the evaluation requirements for the optionalInterface
  /// (interface_set) and the subIterator (sub_iterator_set) from the
  /// total model evaluation requirements (mapped_set)
  void set_mapping(const ActiveSet& mapped_set, ActiveSet& interface_set,
		   bool& opt_interface_map,     ActiveSet& sub_iterator_set,
		   bool& sub_iterator_map);

  /// combine the response from the optional interface evaluation with the
  /// response from the sub-iteration using the primaryCoeffs/secondaryCoeffs
  /// mappings to create the total response for the model
  void response_mapping(const Response& interface_response,
			const Response& sub_iterator_response,
			Response& mapped_response);
  /// assign the response from the optional interface evaluation
  /// within the total response for the model
  void interface_response_overlay(const Response& opt_interface_response,
				  Response& mapped_response);
  /// overlay the sub-iteration response within the total response for
  /// the model using the primaryCoeffs/secondaryCoeffs mappings
  void iterator_response_overlay(const Response& sub_iterator_response,
				 Response& mapped_response);
  /// combine error estimates from the sub-iteration to define
  /// mappedErrorEstimates
  void iterator_error_estimation(const RealVector& sub_iterator_errors,
				 RealVector& mapped_errors);

  /// locate existing or allocate new entry in nestedResponseMap
  Response& nested_response(int nested_cntr);
  /// check function counts for the mapped_asv
  void check_response_map(const ShortArray& mapped_asv);

  /// update inactive variables view for subIterator based on new_view
  void update_inactive_view(short new_view, short& view);
  /// update inactive variables view for subIterator based on type
  void update_inactive_view(unsigned short type, short& view);

  /// update subModel with current variable values/bounds/labels
  void update_sub_model(const Variables& vars, const Constraints& cons);

  //
  //- Heading: Data members
  //

  /// number of calls to derived_evaluate()/derived_evaluate_nowait()
  int nestedModelEvalCntr;
  /// boolean to trigger one-time updates on first call to update_sub_model()
  bool firstUpdate;

  /// used to return a map of nested responses (including subIterator
  /// and optionalInterface contributions) for aggregation and rekeying
  /// at the base class level
  IntResponseMap nestedResponseMap;

  /// mapping of subIterator.response_error_estimates() through
  /// primary and secondary mappings
  RealVector mappedErrorEstimates;

  /// the miPLIndex for the outer parallelism context, prior to any
  /// subIterator partitioning
  size_t outerMIPLIndex;

  // attributes pertaining to the subIterator/subModel pair:
  //
  /// the sub-iterator that is executed on every evaluation of this model
  Iterator subIterator;
  /// the sub-method pointer from the nested model specification
  String subMethodPointer;
  /// the sub-model used in sub-iterator evaluations
  /** There are no restrictions on subModel, so arbitrary nestings are
      possible.  This is commonly used to support surrogate-based
      optimization under uncertainty by having NestedModels contain
      SurrogateModels and vice versa. */
  Model subModel;
  /// job queue for asynchronous execution of subIterator jobs
  PRPQueue subIteratorPRPQueue;
  /// scheduling object for concurrent iterator parallelism
  IteratorScheduler subIteratorSched;
  /// subIterator job counter since last synchronize()
  int subIteratorJobCntr;
  /// mapping from subIterator evaluation counter to nested model counter
  /// (different when subIterator evaluations do not occur on every nested
  /// model evaluation due to variable ASV content)
  IntIntMap subIteratorIdMap;
  /// number of sub-iterator response functions prior to mapping
  size_t numSubIterFns;
  /// number of top-level inequality constraints mapped from the
  /// sub-iteration results
  size_t numSubIterMappedIneqCon;
  /// number of top-level equality constraints mapped from the
  /// sub-iteration results
  size_t numSubIterMappedEqCon;

  // Attributes pertaining to optionalInterface:
  //
  /// the optional interface contributes nonnested response data to
  /// the total model response
  Interface optionalInterface;
  /// the optional interface pointer from the nested model specification
  String optInterfacePointer;
  /// the response object resulting from optional interface evaluations
  Response optInterfaceResponse;
  /// mapping from optionalInterface evaluation counter to nested model
  /// counter (different when optionalInterface evaluations do not occur
  /// on every nested model evaluation due to variable ASV content)
  IntIntMap optInterfaceIdMap;
  /// number of primary response functions (objective/least squares/generic
  /// functions) resulting from optional interface evaluations
  size_t numOptInterfPrimary;
  /// number of inequality constraints resulting from optional
  /// interface evaluations
  size_t numOptInterfIneqCon;
  /// number of equality constraints resulting from the optional
  /// interface evaluations
  size_t numOptInterfEqCon;

  // Attributes pertaining to variables mapping
  //
  /// "primary" variable mappings for inserting active continuous
  /// currentVariables within all continuous subModel variables.  If
  /// there are no secondary mappings defined, then the insertions
  /// replace the subModel variable values.
  SizetArray active1ACVarMapIndices;
  /// "primary" variable mappings for inserting active discrete int
  /// currentVariables within all discrete int subModel variables.  No
  /// secondary mappings are defined for discrete int variables, so the
  /// active variables replace the subModel variable values.
  SizetArray active1ADIVarMapIndices;
  /// "primary" variable mappings for inserting active discrete string
  /// currentVariables within all discrete string subModel variables.  No
  /// secondary mappings are defined for discrete string variables, so the
  /// active variables replace the subModel variable values.
  SizetArray active1ADSVarMapIndices;
  /// "primary" variable mappings for inserting active discrete real
  /// currentVariables within all discrete real subModel variables.  No
  /// secondary mappings are defined for discrete real variables, so the
  /// active variables replace the subModel variable values.
  SizetArray active1ADRVarMapIndices;
  //
  /// "secondary" variable mappings for inserting active continuous
  /// currentVariables into sub-parameters (e.g., distribution parameters
  /// for uncertain variables or bounds for continuous design/state
  /// variables) within all continuous subModel variables.
  ShortArray active2ACVarMapTargets;
  /// "secondary" variable mappings for inserting active discrete int
  /// currentVariables into sub-parameters (e.g., bounds for discrete
  /// design/state variables) within all discrete int subModel variables.
  ShortArray active2ADIVarMapTargets;
  /// "secondary" variable mappings for inserting active discrete string
  /// currentVariables into sub-parameters (e.g., bounds for discrete
  /// design/state variables) within all discrete string subModel variables.
  ShortArray active2ADSVarMapTargets;
  /// "secondary" variable mappings for inserting active discrete real
  /// currentVariables into sub-parameters (e.g., bounds for discrete
  /// design/state variables) within all discrete real subModel variables.
  ShortArray active2ADRVarMapTargets;
  //
  /// "primary" variable mappings for inserting the complement of the active
  /// continuous currentVariables within all continuous subModel variables
  SizetArray complement1ACVarMapIndices;
  /// "primary" variable mappings for inserting the complement of the active
  /// discrete int currentVariables within all discrete int subModel variables
  SizetArray complement1ADIVarMapIndices;
  /// "primary" variable mappings for inserting the complement of the active
  /// discrete string currentVariables within all discrete string subModel
  /// variables
  SizetArray complement1ADSVarMapIndices;
  /// "primary" variable mappings for inserting the complement of the active
  /// discrete real currentVariables within all discrete real subModel variables
  SizetArray complement1ADRVarMapIndices;
  //
  /// flags for updating subModel continuous bounds and labels, one
  /// for each active continuous variable in currentVariables
  BitArray extraCVarsData;
  /// flags for updating subModel discrete int bounds and labels, one
  /// for each active discrete int variable in currentVariables
  BitArray extraDIVarsData;
  /// flags for updating subModel discrete string labels, one for each
  /// active discrete string variable in currentVariables
  BitArray extraDSVarsData;
  /// flags for updating subModel discrete real bounds and labels, one
  /// for each active discrete real variable in currentVariables
  BitArray extraDRVarsData;

  // Attributes pertaining to response_mapping (NOTE: these are opt.
  // specific for now -> generalize for UOO and others):
  //
  /// "primary" response_mapping matrix applied to the sub-iterator response
  /// functions.  For OUU, the matrix is applied to UQ statistics to create
  /// contributions to the top-level objective functions/least squares/
  /// generic response terms.
  RealMatrix primaryRespCoeffs;
  /// "secondary" response_mapping matrix applied to the sub-iterator response
  /// functions.  For OUU, the matrix is applied to UQ statistics to create
  /// contributions to the top-level inequality and equality constraints.
  RealMatrix secondaryRespCoeffs;
};


inline NestedModel::~NestedModel()
{ } // Virtual destructor handles referenceCount at Strategy level.


inline Iterator& NestedModel::subordinate_iterator()
{ return subIterator; }


inline Model& NestedModel::subordinate_model()
{ return subModel; }


inline void NestedModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  ml.push_back(subModel);
  if (recurse_flag)
    subModel.derived_subordinate_models(ml, true);
}


inline Interface& NestedModel::derived_interface()
{ return optionalInterface; }


inline const RealVector& NestedModel::error_estimates()
{
  // For now, assume no error contributions from optional interface, e.g.,
  // these are deterministic mappings and have no estimator variance.

  // *** TO DO: integrate with evaluate and evalaute_nowait()

  iterator_error_estimation(subIterator.response_error_estimates(),
			    mappedErrorEstimates);
  return mappedErrorEstimates; 
}


inline void NestedModel::surrogate_response_mode(short mode)
{ if (mode == BYPASS_SURROGATE) subModel.surrogate_response_mode(mode); }


/** Used in setting Model::asynchEvalFlag.  subModel synchronization
    is used for setting asynchEvalFlag within subModel. */
inline short NestedModel::local_eval_synchronization()
{
  return ( optInterfacePointer.empty() ||
	   optionalInterface.asynch_local_evaluation_concurrency() == 1 ) ?
    SYNCHRONOUS_INTERFACE : optionalInterface.interface_synchronization();
}


/** Used in setting Model::evaluationCapacity.  subModel concurrency
    is used for setting evaluationCapacity within subModel. */
inline int NestedModel::local_eval_concurrency()
{
  return ( optInterfacePointer.empty() ) ? 0 :
    optionalInterface.asynch_local_evaluation_concurrency();
}


/** Derived master overload for subModel is handled separately in
    subModel.evaluate() within subIterator.run(). */
inline bool NestedModel::derived_master_overload() const
{
  bool oi_overload = ( !optInterfacePointer.empty() &&
		       optionalInterface.iterator_eval_dedicated_master() && 
		       optionalInterface.multi_proc_eval() ),
    si_overload = ( !subIterator.is_null() &&
		    subIteratorSched.iteratorScheduling == MASTER_SCHEDULING && 
		    subIteratorSched.procsPerIterator > 1 );
  return (oi_overload || si_overload);
}


inline IntIntPair NestedModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // extract scheduling data for this level prior to dive
  int ppi       = probDescDB.get_int("model.nested.processors_per_iterator"),
    i_servers   = probDescDB.get_int("model.nested.iterator_servers");
  short i_sched = probDescDB.get_short("model.nested.iterator_scheduling");

  int oi_min_procs, oi_max_procs;
  if (optInterfacePointer.empty())
    oi_min_procs = oi_max_procs = 1;
  else {
    oi_min_procs = probDescDB.min_procs_per_ie();
    oi_max_procs = probDescDB.max_procs_per_ie(max_eval_concurrency);
  }

  String empty_str;
  subIteratorSched.construct_sub_iterator(probDescDB, subIterator, subModel,
    subMethodPointer, empty_str, empty_str);
  IntIntPair min_max, si_min_max = subIterator.estimate_partition_bounds();

  // apply multiplier from concurrent iterator scheduling overrides
  min_max.first = ProblemDescDB::min_procs_per_level(
    std::min(oi_min_procs, si_min_max.first), ppi, i_servers);
  min_max.second = ProblemDescDB::max_procs_per_level(
    std::max(oi_max_procs, si_min_max.second), ppi, i_servers, i_sched, 1,
    false, max_eval_concurrency);
  return min_max;
}


inline void NestedModel::derived_init_serial()
{
  // serial instantiation of subIterator
  size_t method_index = probDescDB.get_db_method_node(),
         model_index  = probDescDB.get_db_model_node(); // for restoration
  probDescDB.set_db_list_nodes(subMethodPointer);       // even if empty
  subIterator = probDescDB.get_iterator(subModel);
  probDescDB.set_db_method_node(method_index); // restore method only
  probDescDB.set_db_model_nodes(model_index);  // restore all model nodes

  update_sub_iterator();

  // initialize optionalInterface and subModel for serial operations
  // (e.g., num servers = 1 instead of the 0 default used by
  // ParallelLibrary::resolve_inputs())
  if (!optInterfacePointer.empty())
    optionalInterface.init_serial();
  subModel.init_serial();
}


inline size_t NestedModel::mi_parallel_level_index() const
{ return subIteratorSched.miPLIndex; }


inline void NestedModel::stop_servers()
{ component_parallel_mode(0); }


inline Response& NestedModel::nested_response(int nested_cntr)
{
  IntRespMIter r_it = nestedResponseMap.find(nested_cntr);
  if (r_it == nestedResponseMap.end()) {
    //nestedVarsMap[nestedModelEvalCntr] = currentVariables.copy();
    Response& nested_resp = nestedResponseMap[nested_cntr]; // empty handle
    nested_resp = currentResponse.copy(); nested_resp.reset();
    return nested_resp;
  }
  else
    return r_it->second;
}


/** In the OUU case,
\verbatim
optionalInterface fns = {f}, {g} (deterministic primary functions, constraints)
subIterator fns       = {S}      (UQ response statistics)

Problem formulation for mapped functions:
                  minimize    {f} + [W]{S}
                  subject to  {g_l} <= {g}    <= {g_u}
                              {a_l} <= [A]{S} <= {a_u}
                              {g}    == {g_t}
                              [A]{S} == {a_t}
\endverbatim

where [W] is the primary_mapping_matrix user input (primaryRespCoeffs
class attribute), [A] is the secondary_mapping_matrix user input
(secondaryRespCoeffs class attribute), {{g_l},{a_l}} are the top level
inequality constraint lower bounds, {{g_u},{a_u}} are the top level
inequality constraint upper bounds, and {{g_t},{a_t}} are the top
level equality constraint targets.

NOTE: optionalInterface/subIterator primary fns (obj/lsq/generic fns)
overlap but optionalInterface/subIterator secondary fns (ineq/eq 
constraints) do not.  The [W] matrix can be specified so as to allow

\li some purely deterministic primary functions and some combined:
    [W] filled and [W].num_rows() < {f}.length() [combined first] 
    \e or [W].num_rows() == {f}.length() and [W] contains rows of 
    zeros [combined last]
\li some combined and some purely stochastic primary functions:
    [W] filled and [W].num_rows() > {f}.length()
\li separate deterministic and stochastic primary functions:
    [W].num_rows() > {f}.length() and [W] contains {f}.length()
    rows of zeros.

If the need arises, could change constraint definition to allow overlap
as well: {g_l} <= {g} + [A]{S} <= {g_u} with [A] usage the same as for
[W] above.

In the UOO case, things are simpler, just compute statistics of each 
optimization response function: [W] = [I], {f}/{g}/[A] are empty. */
inline void NestedModel::
response_mapping(const Response& opt_interface_response,
		 const Response& sub_iterator_response,
		 Response& mapped_response)
{
  check_response_map(mapped_response.active_set_request_vector());
  interface_response_overlay(opt_interface_response, mapped_response);
  iterator_response_overlay(sub_iterator_response, mapped_response);
}


inline const String& NestedModel::interface_id() const
{ return optionalInterface.interface_id(); }


/** return the top level nested evaluation count.  To get the lower level
    eval count, the subModel must be explicitly queried.  This is
    consistent with the eval counter definitions in surrogate models. */
inline int NestedModel::derived_evaluation_id() const
{ return nestedModelEvalCntr; }


inline void NestedModel::set_evaluation_reference()
{
  if (!optInterfacePointer.empty())
    optionalInterface.set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //subModel.set_evaluation_reference();

  // may want to add this in time
  //nestedModelEvalRef = nestedModelEvalCntr;
}


inline void NestedModel::fine_grained_evaluation_counters()
{
  if (!optInterfacePointer.empty()) {
    size_t num_oi_fns
      = numOptInterfPrimary + numOptInterfIneqCon + numOptInterfEqCon;
    optionalInterface.fine_grained_evaluation_counters(num_oi_fns);
  }
  subModel.fine_grained_evaluation_counters();
}


inline void NestedModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  if (!optInterfacePointer.empty())
    optionalInterface.print_evaluation_summary(s, minimal_header,
					       relative_count);
  // subIterator will reset evaluation references, so do not use relative counts
  subModel.print_evaluation_summary(s, minimal_header, false);
}


inline void NestedModel::warm_start_flag(const bool flag)
{
  warmStartFlag = flag;
  subModel.warm_start_flag(flag);
}


inline PRPQueueIter NestedModel::job_index_to_queue_iterator(int job_index)
{
  // This approach is insufficient on remote servers with local PRPQueues:
  //PRPQueueIter prp_it = subIteratorPRPQueue.begin();
  //std::advance(prp_it, job_index);

  // map from job_index to nestedModelEvalCntr:
  IntIntMIter id_it = subIteratorIdMap.find(job_index+1); // index to id
  if (id_it == subIteratorIdMap.end()) {
    Cerr << "Error: map lookup failure for job index " << job_index
	 << " in NestedModel::job_index_to_queue_iterator()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  // map from nestedModelEvalCntr to queue iterator:
  PRPQueueIter q_it = lookup_by_eval_id(subIteratorPRPQueue, id_it->second);
  if (q_it == subIteratorPRPQueue.end()) {
    Cerr << "Error: queue lookup failure for evaluation id " << id_it->second
	 << " in NestedModel::job_index_to_queue_iterator()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return q_it;
}


inline void NestedModel::initialize_iterator(int job_index)
{
  PRPQueueIter q_it = job_index_to_queue_iterator(job_index);
  initialize_iterator(q_it->variables(), q_it->active_set(), q_it->eval_id());
}


inline void NestedModel::
initialize_iterator(const Variables& vars, const ActiveSet& set, int eval_id)
{
  update_sub_model(vars, userDefinedConstraints);
  subIterator.response_results_active_set(set);
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' +
      boost::lexical_cast<String>(eval_id); // unique id from nested eval cntr
    subIterator.eval_tag_prefix(eval_tag);
  }
}


inline void NestedModel::
pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  PRPQueueIter q_it = job_index_to_queue_iterator(job_index);
  send_buffer << q_it->variables() << q_it->active_set() << q_it->eval_id();
}


inline void NestedModel::
unpack(MPIUnpackBuffer& recv_buffer, int job_index, Variables& vars,
       ActiveSet& set, int& eval_id)
{
  recv_buffer >> vars >> set >> eval_id;

  // map from job id (index+1) to nestedModelEvalCntr (insert or overwrite)
  subIteratorIdMap[job_index+1] = eval_id;

  // add new job to local queue
  Response resp = subIterator.response_results().copy();
  resp.active_set(set);
  ParamResponsePair pair(vars, subIterator.method_id(), resp, eval_id, false);
  subIteratorPRPQueue.insert(pair);
}


inline void NestedModel::
unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{
  Variables vars; ActiveSet set; int eval_id;
  unpack(recv_buffer, job_index, vars, set, eval_id);
}


inline void NestedModel::
unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer, int job_index)
{
  Variables vars; ActiveSet set; int eval_id;
  unpack(recv_buffer, job_index, vars, set, eval_id);
  initialize_iterator(vars, set, eval_id);
}


inline void NestedModel::
pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  PRPQueueIter q_it = job_index_to_queue_iterator(job_index);
  send_buffer << q_it->response();

  // local job complete: clean up
  subIteratorIdMap.erase(job_index+1);
  subIteratorPRPQueue.erase(q_it);
}


inline void NestedModel::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{
  PRPQueueIter q_it = job_index_to_queue_iterator(job_index);

  // Bypassing PRPQueue const-ness is OK for the PRP response since this
  // should not affect hash-by-value ordering
  Response resp = q_it->response(); // shallow copy
  recv_buffer >> resp;
}


inline void NestedModel::update_local_results(int job_index)
{
  PRPQueueIter q_it = job_index_to_queue_iterator(job_index);

  // Can't do this since it affects Queue hash-by-value ordering
  //q_it->variables(subIterator.variables_results());

  // Bypassing PRPQueue const-ness is OK for the PRP response since
  // this should not affect hash-by-value ordering
  Response resp = q_it->response(); // shallow copy
  resp.update(subIterator.response_results());
}

} // namespace Dakota

#endif
