/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SimulationModel
//- Description: A simulation-based model that maps variables into responses
//-              using an application interface.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SimulationModel.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef SIMULATION_MODEL_H
#define SIMULATION_MODEL_H

#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "ParallelLibrary.hpp"


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
  ~SimulationModel();                         ///< destructor
    
protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// return userDefinedInterface
  Interface& derived_interface();

  // Perform the response computation portions specific to this derived 
  // class.  In this case, it simply employs userDefinedInterface.map()/
  // synch()/synch_nowait()
  //
  /// portion of evaluate() specific to SimulationModel
  /// (invokes a synchronous map() on userDefinedInterface)
  void derived_evaluate(const ActiveSet& set);
  /// portion of evaluate_nowait() specific to SimulationModel
  /// (invokes an asynchronous map() on userDefinedInterface)
  void derived_evaluate_nowait(const ActiveSet& set);
  /// portion of synchronize() specific to SimulationModel
  /// (invokes synch() on userDefinedInterface)
  const IntResponseMap& derived_synchronize();
  /// portion of synchronize_nowait() specific to SimulationModel
  /// (invokes synch_nowait() on userDefinedInterface)
  const IntResponseMap& derived_synchronize_nowait();

  /// SimulationModel only supports parallelism in userDefinedInterface,
  /// so this virtual function redefinition is simply a sanity check.
  void component_parallel_mode(short mode);

  /// return userDefinedInterface synchronization setting
  short local_eval_synchronization();
  /// return userDefinedInterface asynchronous evaluation concurrency
  int local_eval_concurrency();
  /// flag which prevents overloading the master with a multiprocessor
  /// evaluation (request forwarded to userDefinedInterface)
  bool derived_master_overload() const;

  IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// set up SimulationModel for parallel operations (request forwarded to
  /// userDefinedInterface)
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);
  /// set up SimulationModel for serial operations (request forwarded to
  /// userDefinedInterface).
  void derived_init_serial();
  /// set active parallel configuration for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag = true);
  // deallocate communicator partitions for the SimulationModel
  // (request forwarded to userDefinedInterface)
  //void derived_free_communicators(ParLevLIter pl_iter,
  //                                int max_eval_concurrency,
  //                                bool recurse_flag = true);

  /// Service userDefinedInterface job requests received from the master.
  /// Completes when a termination message is received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);
  /// executed by the master to terminate userDefinedInterface server
  /// operations when SimulationModel iteration is complete.
  void stop_servers();

  /// return the userDefinedInterface identifier
  const String& interface_id() const;
  /// return the current evaluation id for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  int evaluation_id() const;
  /// return flag indicated usage of an evaluation cache by the SimulationModel
  /// (request forwarded to userDefinedInterface)
  bool evaluation_cache() const;

  /// set the evaluation counter reference points for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  void set_evaluation_reference();
  /// request fine-grained evaluation reporting within the userDefinedInterface
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the SimulationModel
  /// (request forwarded to userDefinedInterface)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const;

  /// set the hierarchical eval ID tag prefix
  void eval_tag_prefix(const String& eval_id_str);
  
private:

  //
  //- Heading: Convenience member functions
  //

  /// process the solution level inputs to define solnControlVarIndex,
  /// solnControlVarType, and solnControlCostMap
  void initialize_solution_control(const String& control,
				   const RealVector& cost);

  //
  //- Heading: Data members
  //

  /// the interface used for mapping variables to responses
  Interface userDefinedInterface;

  /// index of the inactive discrete variable that controls the set/range of
  /// solution levels
  size_t solnControlVarIndex;
  /// type of the inactive discrete variable that controls the set/range of
  /// solution levels
  short solnControlVarType;
  /// sorted array of relative costs associated with a set of solution levels
  std::map<Real, size_t> solnControlCostMap;
};


inline SimulationModel::~SimulationModel()
{ } // Virtual destructor handles referenceCount at Strategy level.


inline Interface& SimulationModel::derived_interface()
{ return userDefinedInterface; }


inline void SimulationModel::derived_evaluate(const ActiveSet& set)
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  userDefinedInterface.map(currentVariables, set, currentResponse);

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
}


inline void SimulationModel::derived_evaluate_nowait(const ActiveSet& set)
{ userDefinedInterface.map(currentVariables, set, currentResponse, true); }


inline const IntResponseMap& SimulationModel::derived_synchronize()
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  const IntResponseMap& resp_map = userDefinedInterface.synch();

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
  return resp_map;
}


inline const IntResponseMap& SimulationModel::derived_synchronize_nowait()
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  const IntResponseMap& resp_map = userDefinedInterface.synch_nowait();

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
  return resp_map;
}


inline short SimulationModel::local_eval_synchronization()
{
  return ( userDefinedInterface.asynch_local_evaluation_concurrency() == 1 ) ?
    SYNCHRONOUS_INTERFACE : userDefinedInterface.interface_synchronization();
}


inline int SimulationModel::local_eval_concurrency()
{ return userDefinedInterface.asynch_local_evaluation_concurrency(); }


inline bool SimulationModel::derived_master_overload() const
{
  return ( userDefinedInterface.iterator_eval_dedicated_master() && 
           userDefinedInterface.multi_proc_eval() ) ? true : false;
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
  userDefinedInterface.init_communicators(messageLengths, max_eval_concurrency);
}


inline void SimulationModel::derived_init_serial()
{ userDefinedInterface.init_serial(); }


inline void SimulationModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  parallelLib.parallel_configuration_iterator(modelPCIter);
  userDefinedInterface.set_communicators(messageLengths, max_eval_concurrency);
  set_ie_asynchronous_mode(max_eval_concurrency);// asynchEvalFlag, evalCapacity
}


/*
inline void SimulationModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  // allow recursion to progress - don't store/set/restore
  parallelLib.parallel_configuration_iterator(modelPCIter);
  userDefinedInterface.free_communicators();
}
*/


inline void SimulationModel::
serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false);// no recursion (moot)

  userDefinedInterface.serve_evaluations();
}


inline void SimulationModel::stop_servers()
{
  // store/set/restore ParallelLibrary::currPCIter to simplify recursion
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(modelPCIter);

  userDefinedInterface.stop_evaluation_servers();

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
}


inline const String& SimulationModel::interface_id() const
{ return userDefinedInterface.interface_id(); }


inline int SimulationModel::evaluation_id() const
{ return userDefinedInterface.evaluation_id(); }


inline bool SimulationModel::evaluation_cache() const
{ return userDefinedInterface.evaluation_cache(); }


inline void SimulationModel::set_evaluation_reference()
{ userDefinedInterface.set_evaluation_reference(); }


inline void SimulationModel::fine_grained_evaluation_counters()
{ userDefinedInterface.fine_grained_evaluation_counters(numFns); }


inline void SimulationModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  userDefinedInterface.print_evaluation_summary(s, minimal_header,
						relative_count);
}

} // namespace Dakota

#endif
