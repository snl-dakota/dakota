/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       IteratorScheduler
//- Description: Implementation code for the IteratorScheduler class
//- Owner:       Mike Eldred
//- Checked by:

#include "IteratorScheduler.hpp"
#include "DakotaIterator.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: IteratorScheduler.cpp 6492 2009-12-19 00:04:28Z briadam $";


namespace Dakota {

/** Current constructor parameters are the input specification
    components, which are requests subject to override by
    ParallelLibrary::init_iterator_communicators(). */
IteratorScheduler::
IteratorScheduler(ParallelLibrary& parallel_lib, bool peer_assign_jobs,
		  int num_servers, int procs_per_iterator, short scheduling):
  parallelLib(parallel_lib), numIteratorJobs(1),
  numIteratorServers(num_servers), procsPerIterator(procs_per_iterator),
  iteratorCommRank(0), iteratorCommSize(1), iteratorServerId(0),
  messagePass(false), iteratorScheduling(scheduling),//maxIteratorConcurrency(1)
  peerAssignJobs(peer_assign_jobs), paramsMsgLen(0), resultsMsgLen(0)
{
  // Supported examples of a single level of concurrent iterators:
  //   ConcurrentMinimizer (multi_start, pareto_set), BranchBndMinimizer
  //   --> explicit parallelism management at Strat level
  //
  // Single level examples not yet supported:
  //   Reliability UQ with concurrent MPP searches
  //   OUU1, SBOUU2, MDO with single sub-method, other nested model uses
  //   --> single use of NestedModel, expanded iterator support
  //   --> could use existing iterator_servers strategy spec as part of OUU,
  //       SBO, or single_method spec.
  //
  // Advanced examples with multiple levels of concurrent iterators:
  //   MINLP OUU
  //   RBDO with concurrent MPP searches
  //   MDO with multiple sub-iterators
  //   --> NestedModel: multiple nestings or multiple sub-methods
  //   --> can't support with existing iterator_servers strategy spec
}


/** Called from derived class constructors once maxIteratorConcurrency is
    defined but prior to instantiating Iterators and Models. */
void IteratorScheduler::
init_iterator_parallelism(int max_iterator_concurrency,
			  int min_procs_per_iterator,
			  int max_procs_per_iterator, short default_config)
{
  //maxIteratorConcurrency = max_iterator_concurrency;
  //minProcsPerIterator    = min_procs_per_iterator;
  //maxProcsPerIterator    = (max_procs_per_iterator) ?
  //  max_procs_per_iterator : parallelLib.world_size();
  if (!max_procs_per_iterator)
    max_procs_per_iterator = parallelLib.world_size();

  // Default parallel config for concurrent iterators is currently PUSH_DOWN:
  // >> parallel B&B has idle servers on initial phases.  When iterator servers
  //    are specified, the default max_iterator_concurrency yields a peer
  //    partition for use by distributed scheduling.
  // >> For multi_start/pareto_set, optimizer durations are likely to be large
  //    blocks with high variability; therefore, serialize at the source of the
  //    variability (PUSH_DOWN).
  // Concurrent iterator approach that might work well with PUSH_UP:
  // >> OUU/Mixed UQ if the aleatory UQ used a fixed number of samples

  // Initialize iterator partitions after parsing but prior to output/restart.
  // The default setting for max_iterator_concurrency is the number of specified
  // iterator servers, which will yield a peer partition.
  const ParallelLevel& mi_pl = parallelLib.init_iterator_communicators(
    numIteratorServers, procsPerIterator, min_procs_per_iterator,
    max_procs_per_iterator, max_iterator_concurrency, default_config,
    iteratorScheduling, false);// peer_dynamic not available prior to threading

  // new level has been appended to ParallelLibrary::currPCIter
  ParConfigLIter pc_iter = parallelLib.parallel_configuration_iterator();
  update(pc_iter->mi_parallel_level_last_index());

  // Manage ostream output and binary restart input/output.  If concurrent
  // iterators, use tagged files.
  // *** Since only iterator masters manage output/restart, a simple bcast is
  // *** inefficient and iterator partitions are used.  Moreover, these
  // *** partitions must exist to define the iterator masters for performing
  // *** output/restart.  Therefore, must follow iterator partitioning and
  // *** precede iterator instantiation to control output.
  // This tagging is only done for the top level of concurrent iterators at the
  // strategy level.  For concurrent iterators within a nested model, iterator
  // output will be suppressed but restart will need some additional attention.
  // -->> second level of tagging?  e.g., dakota.rst.1.3?
  parallelLib.push_output_tag(mi_pl);
}


/* Called for serialization of concurrent-iterator parallelism levels.
void IteratorScheduler::init_serial_iterators(ParallelLibrary& parallel_lib)
{
  // This is logically equivalent to init_iterator_parallelism(1), but static
  // declaration allows its use in contexts without an IteratorScheduler
  // instance (e.g., Dakota::Environment).  Since it is static, it does not
  // update IteratorScheduler state.

  // Initialize iterator partitions for one iterator execution at a time
  const ParallelLevel& mi_pl = parallel_lib.init_iterator_communicators(0, 0,
    1, parallel_lib.world_size(), 1, PUSH_DOWN, DEFAULT_SCHEDULING, false);
  // set up output streams without iterator tagging
  parallel_lib.push_output_tag(mi_pl);
}


// Static version:
void IteratorScheduler::
free_iterator_parallelism(ParallelLibrary& parallel_lib)//, size_t index)
{
  // deallocate the mi_pl parallelism level (static version)
  parallel_lib.free_iterator_communicators();//(index);
}
*/


void IteratorScheduler::free_iterator_parallelism()
{
  parallelLib.pop_output_tag(schedPCIter->mi_parallel_level(miPLIndex));

  // deallocate the mi_pl parallelism level
  parallelLib.free_iterator_communicators(miPLIndex);
}


/** This is a convenience function for computing the maximum
    evaluation concurrency prior to concurrent iterator partitioning. */
int IteratorScheduler::
init_evaluation_concurrency(ProblemDescDB& problem_db, Iterator& the_iterator,
			    Model& the_model)
{
  // Prior to concurrent iterator partitioning via IteratorScheduler::
  // init_iterator_parallelism(), we utilize the trailing mi_pl (often
  // the world pl) for the concurrency estimation.  If this is not the
  // correct reference point, the calling code must increment the parallel
  // configuration prior to invocation of this fn.
  const ParallelLevel& mi_pl = schedPCIter->mi_parallel_level(); // last level

  //if (mi_pl.dedicated_master() && mi_pl.server_id() == 0) // ded master proc
  //  return;

  int max_eval_concurrency;
  if (mi_pl.server_communicator_rank() == 0) {
    if (the_iterator.is_null())
      the_iterator = problem_db.get_iterator(the_model);
    max_eval_concurrency = the_iterator.maximum_evaluation_concurrency();
    if (mi_pl.server_communicator_size() > 1)
      parallelLib.bcast(max_eval_concurrency, mi_pl);
  }
  else
    parallelLib.bcast(max_eval_concurrency, mi_pl);
  return max_eval_concurrency;
}


/** This is a convenience function for computing the maximum
    evaluation concurrency prior to concurrent iterator partitioning. */
int IteratorScheduler::
init_evaluation_concurrency(const String& method_string, Iterator& the_iterator,
			    Model& the_model)
{
  // Prior to concurrent iterator partitioning via IteratorScheduler::
  // init_iterator_parallelism(), we utilize the trailing mi_pl (often
  // the world pl) for the concurrency estimation.  If this is not the
  // correct reference point, the calling code must increment the parallel
  // configuration prior to invocation of this fn.
  const ParallelLevel& mi_pl = schedPCIter->mi_parallel_level(); // last level

  //if (mi_pl.dedicated_master() && mi_pl.server_id() == 0) // ded master proc
  //  return;

  int max_eval_concurrency;
  if (mi_pl.server_communicator_rank() == 0) {
    if (the_iterator.is_null())
      the_iterator = Iterator(method_string, the_model);
    max_eval_concurrency = the_iterator.maximum_evaluation_concurrency();
    if (mi_pl.server_communicator_size() > 1)
      parallelLib.bcast(max_eval_concurrency, mi_pl);
  }
  else
    parallelLib.bcast(max_eval_concurrency, mi_pl);
  return max_eval_concurrency;
}


/** This is a convenience function for encapsulating the allocation of
    communicators prior to running an iterator. */
void IteratorScheduler::
init_iterator(ProblemDescDB& problem_db, Iterator& the_iterator,
	      Model& the_model, ParLevLIter pl_iter)
{
  // pl_iter advanced by miPLIndex update() in init_iterator_parallelism()
  if (pl_iter->dedicated_master() && pl_iter->server_id() == 0)//ded master proc
    return;

  // iterator rank 0: Instantiate the iterator and initialize communicators.
  // Logic below based on iteratorCommSize could be more efficient with use of
  // ieCommSplitFlag, but this is not available until after Model::init_comms.
  if (pl_iter->server_communicator_rank() == 0) {
    bool multiproc = (pl_iter->server_communicator_size() > 1);
    if (multiproc) the_model.init_comms_bcast_flag(true);
    // only master processor needs an iterator object:
    if (the_iterator.is_null())
      the_iterator = problem_db.get_iterator(the_model);
    the_iterator.init_communicators(pl_iter);
    if (multiproc) the_model.stop_init(pl_iter);
  }
  // iterator ranks 1->n: match all init_communicators() calls that occur
  // on rank 0 (due both to implicit model recursions within the Iterator
  // constructors and the explicit call above).  Some data is stored in the
  // empty envelope for later use in execute/destruct or run/free_iterator.
  else {
    int last_concurrency = the_model.serve_init(pl_iter);
    // store data for {run,free}_iterator() below:
    the_iterator.maximum_evaluation_concurrency(last_concurrency);
    the_iterator.iterated_model(the_model);
    // store for meta-iterator bit logic applied to all ranks
    // (e.g., Environment::execute/destruct()):
    the_iterator.method_name(problem_db.get_ushort("method.algorithm"));
  }
}


/** This is a convenience function for encapsulating the allocation of
    communicators prior to running an iterator. */
void IteratorScheduler::
init_iterator(const String& method_string, Iterator& the_iterator,
	      Model& the_model, ParLevLIter pl_iter)
{
  // pl_iter advanced by miPLIndex update() in init_iterator_parallelism()
  if (pl_iter->dedicated_master() && pl_iter->server_id() == 0)//ded master proc
    return;

  // iterator rank 0: Instantiate the iterator and initialize communicators.
  // Logic below based on iteratorCommSize could be more efficient with use of
  // ieCommSplitFlag, but this is not available until after Model::init_comms.
  if (pl_iter->server_communicator_rank() == 0) {
    bool multiproc = (pl_iter->server_communicator_size() > 1);
    if (multiproc) the_model.init_comms_bcast_flag(true);
    // only master processor needs an iterator object:
    // Note: problem_db.get_iterator() is not used since method_string
    // is insufficient to distinguish unique from shared instances.
    if (the_iterator.is_null())
      the_iterator = Iterator(method_string, the_model);
    the_iterator.init_communicators(pl_iter);
    if (multiproc) the_model.stop_init(pl_iter);
  }
  // iterator ranks 1->n: match all init_communicators() calls that occur
  // on rank 0 (due both to implicit model recursions within the Iterator
  // constructors and the explicit call above).  Some data is stored in the
  // empty envelope for later use in execute/destruct or run/free_iterator.
  else {
    int last_concurrency = the_model.serve_init(pl_iter);
    // store data for {run,free}_iterator() below:
    the_iterator.maximum_evaluation_concurrency(last_concurrency);
    the_iterator.iterated_model(the_model);
    // store for meta-iterator bit logic applied to all ranks
    // (e.g., Environment::execute/destruct()):
    the_iterator.method_string(method_string);
  }
}


/** This is a convenience function for encapsulating the deallocation
    of communicators after running an iterator. */
void IteratorScheduler::
set_iterator(Iterator& the_iterator, ParLevLIter pl_iter)
{
  if (pl_iter->dedicated_master() && pl_iter->server_id() == 0)//ded master proc
    return;

  // iterator rank 0: set the iterator communicators
  if (pl_iter->server_communicator_rank() == 0)
    the_iterator.set_communicators(pl_iter);
  // iterator ranks 1->n
  else // empty envelope: fwds to iteratedModel using data stored in init_comms
    the_iterator.derived_set_communicators(pl_iter);
    //the_model.serve_set(); // if additional sophistication becomes needed
}


/** This is a convenience function for encapsulating the parallel features
    (run/serve) of running an iterator.  This function omits
    allocation/deallocation of communicators to provide greater efficiency
    in approaches that involve multiple iterator executions but only
    require communicator allocation/deallocation to be performed once. */
void IteratorScheduler::
run_iterator(Iterator& the_iterator, ParLevLIter pl_iter)
{
  // for iterator ranks > 0, the_model is stored in the empty iterator
  // envelope in IteratorScheduler::init_iterator()
  Model& the_model = the_iterator.iterated_model();

  // segregate processors into run/serve
  if (pl_iter->server_communicator_rank() == 0) { // iteratorCommRank
    the_iterator.run(pl_iter); // set_communicators() occurs inside run()
    the_model.stop_servers();  // send termination message to all servers
  }
  else // serve until stopped
    the_model.serve_run(pl_iter, the_iterator.maximum_evaluation_concurrency());
}


/** This is a convenience function for encapsulating the deallocation
    of communicators after running an iterator. */
void IteratorScheduler::
free_iterator(Iterator& the_iterator, ParLevLIter pl_iter)
{
  if (pl_iter->dedicated_master() && pl_iter->server_id() == 0)//ded master proc
    return;

  // iterator rank 0: free the iterator communicators
  if (pl_iter->server_communicator_rank() == 0)
    the_iterator.free_communicators(pl_iter);
  // iterator ranks 1->n
  else // empty envelope: fwds to iteratedModel using data stored in init_comms
    the_iterator.derived_free_communicators(pl_iter);
    //the_model.serve_free(); // if additional sophistication becomes needed
}


void IteratorScheduler::stop_iterator_servers()
{
  // Only used for dedicated master iterator scheduling
  // (see ApplicationInterface::stop_evaluation_servers() for example where
  // termination of both master and peer scheduling are managed)

  // terminate iterator servers
  MPIPackBuffer send_buffer(0); // empty buffer
  MPI_Request   send_request;
  int server_id, term_tag = 0;
  if (iteratorScheduling == MASTER_SCHEDULING)
    for (server_id=1; server_id<=numIteratorServers; ++server_id) {
      // nonblocking sends: master posts all terminate messages without waiting
      // for completion.  Bcast cannot be used since all procs must call it and
      // slaves are using Recv/Irecv in serve_evaluation_synch/asynch.
      parallelLib.isend_mi(send_buffer, server_id, term_tag,
			   send_request, miPLIndex);
      parallelLib.free(send_request); // no test/wait on send_request
    }
  else
    server_id = numIteratorServers;

  // if the communicator partitioning resulted in a trailing partition of 
  // idle processors due to a partitioning remainder (caused by strictly
  // honoring a processors_per_iterator override), then these are not
  // included in numIteratorServers and we quietly free them separately.
  // We assume a single server with all idle processors and a valid
  // inter-communicator (enforced during split).  Peer partitions have one
  // fewer interComm from server 1 to servers 2-n, relative to ded master
  // partitions which have interComms from server 0 to servers 1-n:
  // > dedicated master: server_id = #servers+1 -> interComm[#servers]
  // > peer:             server_id = #servers   -> interComm[#servers-1]
  if (schedPCIter->mi_parallel_level(miPLIndex).idle_partition()) {
    parallelLib.isend_mi(send_buffer, server_id, term_tag,
			 send_request, miPLIndex);
    parallelLib.free(send_request); // no test/wait on send_request
  }
}

} // namespace Dakota
