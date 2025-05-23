/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SIMULATION_MODEL_H
#define SIMULATION_MODEL_H

#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "ParallelLibrary.hpp"
#include "EvaluationStore.hpp"

namespace Dakota {

/// Derived model class which utilizes a simulation-based application
/// interface to map variables into responses.

/** The SimulationModel class is the simplest of the derived model
    classes.  It provides the capabilities of the original Model class,
    prior to the development of surrogate and nested model extensions.
    The derived response computation and synchronization functions
    utilize an application interface to perform the function evaluations. */

class SimulationModel: public Model
{
public:

  //
  //- Heading: Constructor and destructor
  //

  SimulationModel(ProblemDescDB& problem_db); ///< constructor
  
  /// Return the "default" or maximal ActiveSet for the model
  //ActiveSet default_active_set();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// return userDefinedInterface
  std::shared_ptr<Interface> derived_interface() override;
  void derived_interface(std::shared_ptr<Interface>) override;

  /// return size of solnCntlCostMap, optionally enforcing lower bound
  /// of 1 solution level
  size_t solution_levels() const override;

  /// return all cost estimates from solnCntlCostMap
  RealVector solution_level_costs() const override;
  /// return active cost estimate from solnCntlCostMap
  Real solution_level_cost() const override;
  /// activate entry in solnCntlCostMap
  void solution_level_cost_index(size_t cost_index) override;
  /// return active entry in solnCntlCostMap
  size_t solution_level_cost_index() const override;

  /// return solnCntlVarType
  short solution_control_variable_type() const override;
  /// return solnCntlAVIndex
  size_t solution_control_variable_index() const override;
  /// return solnCntlADVIndex
  size_t solution_control_discrete_variable_index() const override;

  /// return a discrete int variable value corresponding to solnCntlADVIndex
  int    solution_level_int_value() const override;
  /// return a discrete string variable value corresponding to solnCntlADVIndex
  String solution_level_string_value() const override;
  /// return a discrete real variable value corresponding to solnCntlADVIndex
  Real   solution_level_real_value() const override;

  /// return costMetadataIndex
  size_t cost_metadata_index() const override;

  // Perform the response computation portions specific to this derived 
  // class.  In this case, it simply employs userDefinedInterface->map()/
  // synch()/synch_nowait()
  //
  /// portion of evaluate() specific to SimulationModel
  /// (invokes a synchronous map() on userDefinedInterface)
  void derived_evaluate(const ActiveSet& set) override;
  /// portion of evaluate_nowait() specific to SimulationModel
  /// (invokes an asynchronous map() on userDefinedInterface)
  void derived_evaluate_nowait(const ActiveSet& set) override;
  /// portion of synchronize() specific to SimulationModel
  /// (invokes synch() on userDefinedInterface)
  const IntResponseMap& derived_synchronize() override;
  /// portion of synchronize_nowait() specific to SimulationModel
  /// (invokes synch_nowait() on userDefinedInterface)
  const IntResponseMap& derived_synchronize_nowait() override;

  // SimulationModel only supports parallelism in userDefinedInterface,
  // so this virtual function redefinition is simply a sanity check.
  //void component_parallel_mode(short mode);

  /// return userDefinedInterface synchronization setting
  short local_eval_synchronization() override;
  /// return userDefinedInterface asynchronous evaluation concurrency
  int local_eval_concurrency() override;
  /// flag which prevents overloading the scheduler with a multiprocessor
  /// evaluation (request forwarded to userDefinedInterface)
  bool derived_scheduler_overload() const override;

  IntIntPair estimate_partition_bounds(int max_eval_concurrency) override;

  /// set up SimulationModel for parallel operations (request forwarded to
  /// userDefinedInterface)
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true) override;
  /// set up SimulationModel for serial operations (request forwarded to
  /// userDefinedInterface).
  void derived_init_serial() override;
  /// set active parallel configuration for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag = true) override;
  // deallocate communicator partitions for the SimulationModel
  // (request forwarded to userDefinedInterface)
  //void derived_free_communicators(ParLevLIter pl_iter,
  //                                int max_eval_concurrency,
  //                                bool recurse_flag = true);

  /// Service userDefinedInterface job requests received from the scheduler.
  /// Completes when a termination message is received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency) override;
  /// executed by the scheduler to terminate userDefinedInterface server
  /// operations when SimulationModel iteration is complete.
  void stop_servers() override;

  /// return the userDefinedInterface identifier
  const String& interface_id() const override;
  /// return the current evaluation id (simModelEvalCntr)
  int derived_evaluation_id() const override;
  /// return flag indicated usage of an evaluation cache by the SimulationModel
  /// (request forwarded to userDefinedInterface)
  bool evaluation_cache(bool recurse_flag = true) const override;
  /// return flag indicated usage of a restart file by the SimulationModel
  /// (request forwarded to userDefinedInterface)
  bool restart_file(bool recurse_flag = true) const override;

  /// set the evaluation counter reference points for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  void set_evaluation_reference() override;
  /// request fine-grained evaluation reporting within the userDefinedInterface
  void fine_grained_evaluation_counters() override;
  /// print the evaluation summary for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const override;

  /// set the hierarchical eval ID tag prefix
  void eval_tag_prefix(const String& eval_id_str) override;

  
  /// Return the "default" or maximal ActiveSet for the userDefinedInterface
  ActiveSet default_interface_active_set();

  /// Declare this model's sources
  void declare_sources() override;

private:

  //
  //- Heading: Convenience member functions
  //

  /// process the solution level inputs to define solnCntlVarType,
  /// solnCntlCostMap, and solnCntl{AV,ADV}Index
  void initialize_solution_control(const String& control,
				   const RealVector& cost);
  /// process the solution level inputs to define solnCntlVarType,
  /// solnCntlCostMap, and solnCntl{AV,ADV}Index
  void initialize_solution_recovery(const String& cost_label);

  //
  //- Heading: Data members
  //

  /// the interface used for mapping variables to responses
  std::shared_ptr<Interface> userDefinedInterface;

  /// type of the discrete variable that controls the set/range of
  /// solution levels
  short solnCntlVarType;
  /// index of the discrete variable (within all view) that controls the
  /// set/range of solution levels
  size_t solnCntlADVIndex;
  /// index of the discrete variable (within all variables / RandomVariables
  /// array) that controls the set/range of solution levels
  size_t solnCntlAVIndex;
  /// sorted array of relative costs associated with a set of solution levels
  std::multimap<Real, size_t> solnCntlCostMap;

  /// index of metadata label used for online cost recovery
  size_t costMetadataIndex;

  /// counter for calls to derived_evaluate()/derived_evaluate_nowait()
  size_t simModelEvalCntr;
  /// map from userDefinedInterface evaluation ids to SimulationModel ids
  /// (may differ in case where the same interface instance is shared by
  /// multiple models)
  IntIntMap simIdMap;
  /// map of simulation-based responses returned by derived_synchronize()
  /// and derived_synchronize_nowait()
  IntResponseMap simResponseMap;
};


inline std::shared_ptr<Interface> SimulationModel::derived_interface()
{ return userDefinedInterface; }

inline void SimulationModel::derived_interface(std::shared_ptr<Interface> di)
{ userDefinedInterface = di; }


/* There is a default solution level (nominal settings) even if no
   solution control is provided */ 
inline size_t SimulationModel::solution_levels() const
{ return solnCntlCostMap.size(); }


inline short SimulationModel::solution_control_variable_type() const
{ return solnCntlVarType; }


inline size_t SimulationModel::solution_control_variable_index() const
{ return solnCntlAVIndex; }


inline size_t SimulationModel::solution_control_discrete_variable_index() const
{ return solnCntlADVIndex; }


inline size_t SimulationModel::cost_metadata_index() const
{ return costMetadataIndex; }


inline void SimulationModel::derived_evaluate(const ActiveSet& set)
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);
  ++simModelEvalCntr;

  if(interfEvaluationsDBState == EvaluationsDBState::UNINITIALIZED)
      interfEvaluationsDBState = evaluationsDB.interface_allocate(modelId, 
          interface_id(), "simulation", currentVariables, currentResponse, 
          default_interface_active_set(), userDefinedInterface->analysis_components());

  userDefinedInterface->map(currentVariables, set, currentResponse);

  if(interfEvaluationsDBState == EvaluationsDBState::ACTIVE) {
    evaluationsDB.store_interface_variables(modelId, interface_id(),
        userDefinedInterface->evaluation_id(), set, currentVariables);
    evaluationsDB.store_interface_response(modelId, interface_id(),
        userDefinedInterface->evaluation_id(), currentResponse);
  }

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
}


inline void SimulationModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++simModelEvalCntr;

  if(interfEvaluationsDBState == EvaluationsDBState::UNINITIALIZED)
    interfEvaluationsDBState = evaluationsDB.interface_allocate(modelId, interface_id(),
        "simulation", currentVariables, currentResponse, default_interface_active_set(), 
        userDefinedInterface->analysis_components());

  userDefinedInterface->map(currentVariables, set, currentResponse, true);

  if(interfEvaluationsDBState == EvaluationsDBState::ACTIVE) 
      evaluationsDB.store_interface_variables(modelId, interface_id(),
      userDefinedInterface->evaluation_id(), set, currentVariables);

  // Even though each evaluate on SimulationModel results in a corresponding
  // Interface mapping, we utilize an id mapping to protect against the case
  // where multiple Models use the same Interface instance, for which this
  // Model instance will only match a subset of the Interface eval ids.
  simIdMap[userDefinedInterface->evaluation_id()] = simModelEvalCntr;
}


inline const IntResponseMap& SimulationModel::derived_synchronize()
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  // Any responses from userDefinedInterface->synchronize() that are unmatched
  // in simIdMap are cached in Interface::cachedResponseMap
  rekey_synch(*userDefinedInterface, true, simIdMap, simResponseMap);
  // Caching for Models must also occur at the base class level
  // (Model::cachedResponseMap) since deriv estimation-based rekeying is
  // performed as this top level (and any lower level mappings are erased
  // as records are rekeyed/promoted, making them unavailable for caching).

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
  return simResponseMap;
}


inline const IntResponseMap& SimulationModel::derived_synchronize_nowait()
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  // See comments above regarding levels of rekeying / caching
  rekey_synch(*userDefinedInterface, false, simIdMap, simResponseMap);

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
  return simResponseMap;
}


inline short SimulationModel::local_eval_synchronization()
{
  return ( userDefinedInterface->asynch_local_evaluation_concurrency() == 1 ) ?
    SYNCHRONOUS_INTERFACE : userDefinedInterface->interface_synchronization();
}


inline int SimulationModel::local_eval_concurrency()
{ return userDefinedInterface->asynch_local_evaluation_concurrency(); }


inline bool SimulationModel::derived_scheduler_overload() const
{
  return ( userDefinedInterface->iterator_eval_dedicated_scheduler() && 
           userDefinedInterface->multi_proc_eval() ) ? true : false;
}


inline IntIntPair SimulationModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // Note: accesses DB data
  // > for use at construct/init_comms time
  // > DB list nodes set by calling context
  return IntIntPair(probDescDB.min_procs_per_ie(), 
		    probDescDB.max_procs_per_ie(max_eval_concurrency));
}


inline void SimulationModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  parallelLib.parallel_configuration_iterator(modelPCIter);
  userDefinedInterface->init_communicators(messageLengths, max_eval_concurrency);
}


inline void SimulationModel::derived_init_serial()
{ userDefinedInterface->init_serial(); }


inline void SimulationModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  parallelLib.parallel_configuration_iterator(modelPCIter);
  userDefinedInterface->set_communicators(messageLengths, max_eval_concurrency);
  set_ie_asynchronous_mode(max_eval_concurrency);// asynchEvalFlag, evalCapacity
}


/*
inline void SimulationModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  parallelLib.parallel_configuration_iterator(modelPCIter);
  userDefinedInterface->free_communicators();
}
*/


inline void SimulationModel::
serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false);// no recursion (moot)

  userDefinedInterface->serve_evaluations();
}


inline void SimulationModel::stop_servers()
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  userDefinedInterface->stop_evaluation_servers();

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
}


inline const String& SimulationModel::interface_id() const
{ return userDefinedInterface->interface_id(); }


inline int SimulationModel::derived_evaluation_id() const
{ return simModelEvalCntr; }


inline bool SimulationModel::evaluation_cache(bool recurse_flag) const
{ return userDefinedInterface->evaluation_cache(); }


inline bool SimulationModel::restart_file(bool recurse_flag) const
{ return userDefinedInterface->restart_file(); }


inline void SimulationModel::set_evaluation_reference()
{ userDefinedInterface->set_evaluation_reference(); }


inline void SimulationModel::fine_grained_evaluation_counters()
{ userDefinedInterface->fine_grained_evaluation_counters(numFns); }


inline void SimulationModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  userDefinedInterface->print_evaluation_summary(s, minimal_header,
						relative_count);
}

} // namespace Dakota

#endif
