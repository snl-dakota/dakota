/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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


namespace Dakota {

// define special values for componentParallelMode
#define OPTIONAL_INTERFACE 1
#define SUB_MODEL          2


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

  // Perform the response computation portions specific to this derived 
  // class.  In this case, this involves running the subIterator on the 
  // subModel.
  //
  /// portion of compute_response() specific to NestedModel
  void derived_compute_response(const ActiveSet& set);
  /// portion of asynch_compute_response() specific to NestedModel
  void derived_asynch_compute_response(const ActiveSet& set);
  // portion of synchronize() specific to NestedModel
  //const IntResponseMap& derived_synchronize();
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

  /// pass a bypass request on to the subModel for any lower-level surrogates
  void surrogate_response_mode(short mode);

  /// update component parallel mode for supporting parallelism in
  /// optionalInterface and subModel
  void component_parallel_mode(short mode);

  // return optionalInterface synchronization setting
  //String local_eval_synchronization();
  // return optionalInterface asynchronous evaluation concurrency
  //int local_eval_concurrency();

  /// flag which prevents overloading the master with a multiprocessor
  /// evaluation (forwarded to optionalInterface)
  bool derived_master_overload() const;
  /// set up optionalInterface and subModel for parallel operations
  void derived_init_communicators(int max_eval_concurrency,
				  bool recurse_flag = true);
  /// set up optionalInterface and subModel for serial operations.
  void derived_init_serial();
  /// set active parallel configuration within subModel
  void derived_set_communicators(int max_eval_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the NestedModel
  /// (forwarded to optionalInterface and subModel)
  void derived_free_communicators(int max_eval_concurrency,
				  bool recurse_flag = true);

  /// Service optionalInterface and subModel job requests received from
  /// the master.  Completes when a termination message is received from
  /// stop_servers().
  void serve(int max_eval_concurrency);
  /// Executed by the master to terminate server operations for subModel and
  /// optionalInterface when iteration on the NestedModel is complete.
  void stop_servers();

  /// return the optionalInterface identifier
  const String& interface_id() const;
  /// Return the current evaluation id for the NestedModel
  int evaluation_id() const;

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

  /// set the hierarchical eval ID tag prefix
  virtual void eval_tag_prefix(const String& eval_id_str);


private:

  //
  //- Heading: Convenience member functions
  //

  /// for a named real mapping, resolve primary index and secondary target
  void resolve_real_variable_mapping(const String& map1, const String& map2,
				     size_t curr_index,
				     short& inactive_sm_view);
  /// for a named integer mapping, resolve primary index and secondary target
  void resolve_integer_variable_mapping(const String& map1, const String& map2,
					size_t curr_index,
					short& inactive_sm_view);

  /// offset pacvm_index based on sacvm_target to create mapped_index
  size_t sm_acv_index_map(size_t  pacvm_index,  short sacvm_target);
  /// offset padivm_index based on sadivm_target to create mapped_index
  size_t sm_adiv_index_map(size_t padivm_index, short sadivm_target);
  /// offset padrvm_index based on sadrvm_target to create mapped_index
  size_t sm_adrv_index_map(size_t padrvm_index, short sadrvm_target);

  /// offset cv_index to create index into aggregated primary/secondary arrays
  size_t cv_index_map(size_t cv_index);
  /// offset div_index to create index into aggregated primary/secondary arrays
  size_t div_index_map(size_t div_index);
  /// offset drv_index to create index into aggregated primary/secondary arrays
  size_t drv_index_map(size_t drv_index);

  /// offset active complement ccv_index to create index into all
  /// continuous arrays
  size_t ccv_index_map(size_t ccv_index);
  /// offset active complement cdiv_index to create index into all
  /// discrete int arrays
  size_t cdiv_index_map(size_t cdiv_index);
  /// offset active complement cdrv_index to create index into all
  /// discrete real arrays
  size_t cdrv_index_map(size_t cdrv_index);

  /// insert r_var into appropriate recipient
  void real_variable_mapping(const Real& r_var, size_t mapped_index,
			     short svm_target);
  /// insert i_var into appropriate recipient
  void integer_variable_mapping(const int& i_var, size_t mapped_index,
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

  /// update inactive variables view for subIterator based on new_view
  void update_inactive_view(short new_view, short& view);
  /// update inactive variables view for subIterator based on type
  void update_inactive_view(unsigned short type, short& view);

  /// update subModel with current variable values/bounds/labels
  void update_sub_model();

  //
  //- Heading: Data members
  //

  /// number of calls to derived_compute_response()/
  /// derived_asynch_compute_response()
  int nestedModelEvalCntr;

  // attributes pertaining to the subIterator/subModel pair:
  //
  /// the sub-iterator that is executed on every evaluation of this model
  Iterator subIterator;
  /// the sub-model used in sub-iterator evaluations
  /** There are no restrictions on subModel, so arbitrary nestings are
      possible.  This is commonly used to support surrogate-based
      optimization under uncertainty by having NestedModels contain
      SurrogateModels and vice versa. */
  Model subModel;
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
  /// insertions replace the subModel variable values.
  SizetArray active1ADIVarMapIndices;
  /// "primary" variable mappings for inserting active discrete real
  /// currentVariables within all discrete real subModel variables.  No
  /// secondary mappings are defined for discrete real variables, so the
  /// insertions replace the subModel variable values.
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
  /// discrete real currentVariables within all discrete real subModel variables
  SizetArray complement1ADRVarMapIndices;
  //
  /// flags for updating subModel continuous bounds and labels, one
  /// for each active continuous variable in currentVariables
  BoolDeque extraCVarsData;
  /// flags for updating subModel discrete int bounds and labels, one
  /// for each active discrete int variable in currentVariables
  BoolDeque extraDIVarsData;
  /// flags for updating subModel discrete real bounds and labels, one
  /// for each active discrete real variable in currentVariables
  BoolDeque extraDRVarsData;

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

  /// cached evalTag Prefix from parents to use at compute_response time
  String evalTagPrefix;
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


inline void NestedModel::surrogate_response_mode(short mode)
{ if (mode == BYPASS_SURROGATE) subModel.surrogate_response_mode(mode); }


/* Used in setting Model::asynchEvalFlag.  subModel synchronization
   is used for setting asynchEvalFlag within subModel. */
//inline String NestedModel::local_eval_synchronization()
//{
//  return ( optInterfacePointer.empty() ||
//	   optionalInterface.asynch_local_evaluation_concurrency() == 1 ) ?
//    String("synchronous") : optionalInterface.interface_synchronization();
//}


/* Used in setting Model::evaluationCapacity.  subModel concurrency
   is used for setting evaluationCapacity within subModel. */
//inline int NestedModel::local_eval_concurrency()
//{
//  return ( !optInterfacePointer.empty() ) ?
//    optionalInterface.asynch_local_evaluation_concurrency() : 0;
//}


/** Derived master overload for subModel is handled separately in
    subModel.compute_response() within subIterator.run(). */
inline bool NestedModel::derived_master_overload() const
{
  return ( !optInterfacePointer.empty() &&
           optionalInterface.iterator_eval_dedicated_master() && 
           optionalInterface.multi_proc_eval() ) ? true : false;
}


/** Asynchronous flags need to be initialized for the subModel.  In
    addition, max_eval_concurrency is the outer level iterator
    concurrency, not the subIterator concurrency that subModel will
    see, and recomputing the message_lengths on the subModel is
    probably not a bad idea either.  Therefore, recompute everything
    on subModel using init_communicators(). */
inline void NestedModel::
derived_init_communicators(int max_eval_concurrency, bool recurse_flag)
{
  // initialize optionalInterface for parallel operations
  if (!optInterfacePointer.empty())
    optionalInterface.init_communicators(messageLengths, max_eval_concurrency);
  // max concurrency does not require a bcast as in Strategy::init_comms
  // since all procs instantiate subIterator
  if (recurse_flag)
    subIterator.init_communicators();
}


inline void NestedModel::derived_init_serial()
{
  // initialize optionalInterface and subModel for serial operations
  // (e.g., num servers = 1 instead of the 0 default used by
  // ParallelLibrary::resolve_inputs())
  if (!optInterfacePointer.empty())
    optionalInterface.init_serial();
  subModel.init_serial();
}


inline void NestedModel::
derived_set_communicators(int max_eval_concurrency, bool recurse_flag)
{
  if (!optInterfacePointer.empty()) {
    parallelLib.parallel_configuration_iterator(modelPCIter);
    optionalInterface.set_communicators(messageLengths, max_eval_concurrency);
  }
  if (recurse_flag)
    subIterator.set_communicators();
}


inline void NestedModel::
derived_free_communicators(int max_eval_concurrency, bool recurse_flag)
{
  if (!optInterfacePointer.empty()) {
    parallelLib.parallel_configuration_iterator(modelPCIter);
    optionalInterface.free_communicators();
  }
  if (recurse_flag)
    subIterator.free_communicators();
}


inline void NestedModel::serve(int max_eval_concurrency)
{
  // don't recurse, as subModel.serve() will set subModel comms
  set_communicators(max_eval_concurrency, false);

  // manage optionalInterface and subModel servers
  componentParallelMode = 1;
  while (componentParallelMode) {
    parallelLib.bcast_i(componentParallelMode);
    if (componentParallelMode == OPTIONAL_INTERFACE &&
	!optInterfacePointer.empty()) {
      parallelLib.parallel_configuration_iterator(modelPCIter);
      optionalInterface.serve_evaluations();
    }
    else if (componentParallelMode == SUB_MODEL)
      subModel.serve(subIterator.maximum_evaluation_concurrency());
  }
}


inline void NestedModel::stop_servers()
{ component_parallel_mode(0); }


inline const String& NestedModel::interface_id() const
{ return optionalInterface.interface_id(); }


/** return the top level nested evaluation count.  To get the lower level
    eval count, the subModel must be explicitly queried.  This is
    consistent with the eval counter definitions in surrogate models. */
inline int NestedModel::evaluation_id() const
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

} // namespace Dakota

#endif
