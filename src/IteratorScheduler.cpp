/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
IteratorScheduler(ParallelLibrary& parallel_lib, int num_servers,
		  int procs_per_iterator, short scheduling):
  parallelLib(parallel_lib), numIteratorJobs(1),
  numIteratorServers(num_servers),                // DataMethod default = 0
  procsPerIterator(procs_per_iterator),           // DataMethod default = 0
  iteratorCommRank(0), iteratorCommSize(1), iteratorServerId(0),
  messagePass(false),
  iteratorScheduling(scheduling) // DataMethod default = DEFAULT_SCHEDULING
  //maxIteratorConcurrency(1)
{
  // TO DO: support for multiple concurrent iterator partitions.
  //
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
init_iterator_parallelism(int max_concurrency, short default_config)
{
  //maxIteratorConcurrency = max_concurrency;

  // TO DO: May want to set procs_per_iterator to a minimum size to accomodate
  // any lower level requests/overrides.  With default_config = PUSH_DOWN,
  // this is not currently needed.
  //int procs_per_iterator = 0;

  // Default parallel config is currently PUSH_DOWN for all strategies:
  // >> parallel B&B has idle servers on initial phases.  When iterator servers
  //    are specified, the default max_concurrency yields a peer partition for
  //    use by distributed scheduling.
  // >> For multi_start/pareto_set, optimizer durations are likely to be large
  //    blocks with high variability; therefore, serialize at the source of the
  //    variability (PUSH_DOWN).
  // Concurrent iterator approaches that might work well with PUSH_UP:
  // >> OUU if the UQ portion was sampling-based with a fixed number of samples
  // >> Multi-start with non-optimization algorithms (e.g., parameter studies).
  //short default_config = PUSH_DOWN;

  // Initialize iterator partitions after parsing but prior to output/restart.
  // The default setting for max_concurrency is the number of specified iterator
  // servers, which will yield a peer partition.
  const ParallelLevel& si_pl = parallelLib.init_iterator_communicators(
    numIteratorServers, procsPerIterator, max_concurrency, default_config,
    iteratorScheduling, false); // peer_dynamic not available prior to threading

  // retrieve the partition data
  //dedicatedMaster = si_pl.dedicated_master();
  messagePass        = si_pl.message_pass();
  iteratorCommRank   = si_pl.server_communicator_rank();
  iteratorCommSize   = si_pl.server_communicator_size();
  iteratorServerId   = si_pl.server_id();

  // update requests with actual
  numIteratorServers = si_pl.num_servers();
  iteratorScheduling = (si_pl.dedicated_master())
                     ? MASTER_SCHEDULING : PEER_SCHEDULING;

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
  parallelLib.manage_outputs_restart(si_pl);
}


/** Static version called for serialization. */
void IteratorScheduler::init_serial_iterators(ParallelLibrary& parallel_lib)
{
  // This is logically equivalent to init_iterator_parallelism(1), but static
  // declaration allows its use in contexts without an IteratorScheduler
  // instance (e.g., Dakota::Environment).  Since it is static, it does not
  // update IteratorScheduler state.

  // Initialize iterator partitions for one iterator execution at a time
  const ParallelLevel& si_pl = parallel_lib.init_iterator_communicators(0, 0,
    1, PUSH_DOWN, DEFAULT_SCHEDULING, false);
  // set up output streams without iterator tagging
  parallel_lib.manage_outputs_restart(si_pl);
}


void IteratorScheduler::free_iterator_parallelism()
{
  // deallocate the si_pl parallelism level
  parallelLib.free_iterator_communicators();
}


/** Static version */
void IteratorScheduler::free_iterator_parallelism(ParallelLibrary& parallel_lib)
{
  // deallocate the si_pl parallelism level (static version)
  parallel_lib.free_iterator_communicators();
}


/** This is a convenience function for encapsulating the allocation of
    communicators prior to running an iterator. */
void IteratorScheduler::
init_iterator(ProblemDescDB& problem_db, Iterator& the_iterator,
	      Model& the_model, const ParallelLevel& pl)
{
  if (pl.dedicated_master() && pl.server_id() == 0) // ded master processor
    return;

  // iterator rank 0: Instantiate the iterator and initialize communicators.
  // Logic below based on iteratorCommSize could be more efficient with use of
  // ieCommSplitFlag, but this is not available until after Model::init_comms.
  if (pl.server_communicator_rank() == 0) {//iteratorCommRank == 0
    bool multiproc = (pl.server_communicator_size() > 1);//iteratorCommSize > 1
    if (multiproc) the_model.init_comms_bcast_flag(true);
    // only master processor needs an iterator object:
    the_iterator = problem_db.get_iterator(the_model); // new ctor chain
    the_model.init_communicators(the_iterator.maximum_evaluation_concurrency());
    if (multiproc) the_model.stop_configurations();
  }
  // iterator ranks 1->n: match all the_model.init_communicators() calls that
  // occur on rank 0 (due both to implicit model recursions within the Iterator
  // constructors and the explicit call above).  Some data is stored in the
  // empty envelope for later use in execute/destruct or run/free_iterator.
  else {
    int last_concurrency = the_model.serve_configurations();
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
	      Model& the_model, const ParallelLevel& pl)
{
  if (pl.dedicated_master() && pl.server_id() == 0) // ded master processor
    return;

  // iterator rank 0: Instantiate the iterator and initialize communicators.
  // Logic below based on iteratorCommSize could be more efficient with use of
  // ieCommSplitFlag, but this is not available until after Model::init_comms.
  if (pl.server_communicator_rank() == 0) {//iteratorCommRank == 0
    bool multiproc = (pl.server_communicator_size() > 1);//iteratorCommSize > 1
    if (multiproc) the_model.init_comms_bcast_flag(true);
    // only master processor needs an iterator object:
    // Note: problem_db.get_iterator() is not used since method_string
    // is insufficient to distinguish unique from shared instances.
    the_iterator = Iterator(method_string, the_model);
    the_model.init_communicators(the_iterator.maximum_evaluation_concurrency());
    if (multiproc) the_model.stop_configurations();
  }
  // iterator ranks 1->n: match all the_model.init_communicators() calls that
  // occur on rank 0 (due both to implicit model recursions within the Iterator
  // constructors and the explicit call above).  Some data is stored in the
  // empty envelope for later use in execute/destruct or run/free_iterator.
  else {
    int last_concurrency = the_model.serve_configurations();
    // store data for {run,free}_iterator() below:
    the_iterator.maximum_evaluation_concurrency(last_concurrency);
    the_iterator.iterated_model(the_model);
    // store for meta-iterator bit logic applied to all ranks
    // (e.g., Environment::execute/destruct()):
    the_iterator.method_string(method_string);
  }
}


/** This is a convenience function for encapsulating the parallel features
    (run/serve) of running an iterator.  This function omits
    allocation/deallocation of communicators to provide greater efficiency
    in approaches that involve multiple iterator executions but only
    require communicator allocation/deallocation to be performed once. */
void IteratorScheduler::
run_iterator(Iterator& the_iterator, const ParallelLevel& pl)
{
  // for iterator ranks > 0, the_model is stored in the empty iterator
  // envelope in IteratorScheduler::init_iterator()
  Model& the_model = the_iterator.iterated_model();

  // set the correct ParallelConfiguration within ParallelLibrary
  //parallelLib.parallel_configuration_iterator(
  //  the_model.parallel_configuration_iterator());

  // segregate processors into run/serve
  if (pl.server_communicator_rank() == 0) { // iteratorCommRank
    the_iterator.run(Cout); // verbose mode
    the_model.stop_servers(); // Send the termination message to the servers
  }
  else {
    //ParConfigLIter prev_pc = parallelLib.parallel_configuration_iterator();
    the_model.serve(the_iterator.maximum_evaluation_concurrency());// until stop
    //parallelLib.parallel_configuration_iterator(prev_pc); // reset
  }
}


/** This is a convenience function for encapsulating the deallocation
    of communicators after running an iterator. */
void IteratorScheduler::
free_iterator(Iterator& the_iterator, const ParallelLevel& pl)
{
  if (pl.dedicated_master() && pl.server_id() == 0) // ded master processor
    return;

  // could replace with new delegation fn:
  //void DakotaIterator::free_communicators()
  //{ iteratedModel.free_communicators(maxEvalConcurrency); }

  // for iterator ranks > 0, the_model is stored in the empty iterator
  // envelope in IteratorScheduler::init_iterator()
  Model& the_model = the_iterator.iterated_model();

  // set the correct ParallelConfiguration within ParallelLibrary
  //parallelLib.parallel_configuration_iterator(
  //  the_model.parallel_configuration_iterator());

  // free fn. evaluation subcommunicators if present.
  the_model.free_communicators(the_iterator.maximum_evaluation_concurrency());
}


/** This implementation supports the scheduling of multiple jobs using
    a single iterator/model pair.  Additional future (overloaded)
    implementations could involve independent iterator instances. */
void IteratorScheduler::
schedule_iterators(Iterator& meta_iterator, Iterator& sub_iterator)
{
  if (iteratorScheduling == MASTER_SCHEDULING) { //(dedicatedMaster) {
    if (iteratorServerId == 0) { // strategy master
      master_dynamic_schedule_iterators(meta_iterator);
      stop_iterator_servers();
    }
    else // slave iterator servers
      serve_iterators(meta_iterator, sub_iterator);
  }
  else // static scheduling of iterator jobs
    // jobs are not assigned by messages -> stop_iterator_servers() is not reqd
    peer_static_schedule_iterators(meta_iterator, sub_iterator);
}


/** This function is adapted from
    ApplicationInterface::master_dynamic_schedule_evaluations(). */
void IteratorScheduler::
master_dynamic_schedule_iterators(Iterator& meta_iterator)
{
  // model.init_communicators() is called on the iterator servers, but not on
  // the strategy master.  Therefore, match collective communications.
  parallelLib.print_configuration(); // matches call within init_communicators()

  int i, num_sends = std::min(numIteratorServers, numIteratorJobs);
  Cout << "Master dynamic schedule: first pass assigning " << num_sends
       << " iterator jobs among " << numIteratorServers << " servers\n";

  MPIPackBuffer*   send_buffers  = new MPIPackBuffer   [num_sends];
  MPIUnpackBuffer* recv_buffers  = new MPIUnpackBuffer [numIteratorJobs];
  MPI_Request      send_request; // only 1 needed since no test/wait on sends
  MPI_Request*     recv_requests = new MPI_Request     [num_sends];

  // assign 1st num_sends jobs
  for (i=0; i<num_sends; i++) {
    // pack the ith parameter set
    //send_buffers[i].resize(paramsMsgLen);
    meta_iterator.pack_parameters_buffer(send_buffers[i], i);
    // pre-post receives
    recv_buffers[i].resize(resultsMsgLen);
    parallelLib.irecv_si(recv_buffers[i], i+1, i+1, recv_requests[i]);
    // nonblocking sends: master quickly assigns first num_sends jobs
    parallelLib.isend_si(send_buffers[i], i+1, i+1, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }

  // schedule remaining jobs
  if (num_sends < numIteratorJobs) {
    Cout << "Master dynamic schedule: second pass scheduling "
	 << numIteratorJobs-num_sends << " remaining iterator jobs\n";
    int send_cntr = num_sends, recv_cntr = 0, out_count;
    MPI_Status* status_array = new MPI_Status [num_sends];
    int*        index_array  = new int        [num_sends];
    while (recv_cntr < numIteratorJobs) {
      parallelLib.waitsome(num_sends, recv_requests, out_count, index_array, 
			   status_array);
      recv_cntr += out_count;
      for (i=0; i<out_count; i++) {
        int server_index = index_array[i]; // index of completed recv_request
        int server_id    = server_index + 1;        // 1 to numIteratorServers
        int job_id       = status_array[i].MPI_TAG; // 1 to numIteratorJobs
        int job_index    = job_id - 1;              // 0 to numIteratorJobs-1
	meta_iterator.unpack_results_buffer(recv_buffers[job_index], job_index);
        if (send_cntr < numIteratorJobs) {
          send_buffers[server_index].reset();// reuse send_buffers/recv_requests
	  meta_iterator.pack_parameters_buffer(send_buffers[server_index],
					      send_cntr);
	  // pre-post receive
          recv_buffers[send_cntr].resize(resultsMsgLen);
          parallelLib.irecv_si(recv_buffers[send_cntr], server_id, send_cntr+1, 
                               recv_requests[server_index]);
          // send next job to open server
          parallelLib.isend_si(send_buffers[server_index], server_id,
                               send_cntr+1, send_request);
          parallelLib.free(send_request); // no test/wait on send_request
          send_cntr++;
        }
      }
    }
    delete [] status_array;
    delete [] index_array;
  }
  else { // all jobs assigned in first pass
    Cout << "Master dynamic schedule: waiting on all iterator jobs."
	 << std::endl;
    parallelLib.waitall(numIteratorJobs, recv_requests);
    // All buffers received, now generate rawResponseArray
    for (i=0; i<numIteratorJobs; i++)
      meta_iterator.unpack_results_buffer(recv_buffers[i], i);
  }
  // deallocate MPI & buffer arrays
  delete [] send_buffers;
  delete [] recv_buffers;
  delete [] recv_requests;
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
  for (server_id=1; server_id<=numIteratorServers; ++server_id) {
    // nonblocking sends: master posts all terminate messages without waiting
    // for completion.  Bcast cannot be used since all procs must call it and
    // slaves are using Recv/Irecv in serve_evaluation_synch/asynch.
    parallelLib.isend_si(send_buffer, server_id, term_tag, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }
  // if the communicator partitioning resulted in a trailing partition of 
  // idle processors due to a partitioning remainder (caused by strictly
  // honoring a processors_per_iterator override), then these are not
  // included in numIteratorServers and we quietly free them separately.
  // We assume a single server with all idle processors and a valid
  // inter-communicator (enforced during split).
  if (parallelLib.parallel_configuration().si_parallel_level().
      idle_partition()) {
    parallelLib.isend_si(send_buffer, server_id, term_tag, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }
}


void IteratorScheduler::
peer_static_schedule_iterators(Iterator& meta_iterator, Iterator& sub_iterator)
{
  const ParallelLevel& si_pl
    = parallelLib.parallel_configuration().si_parallel_level();
  for (int i=iteratorServerId-1; i<numIteratorJobs; i+=numIteratorServers) {

    // Set starting point or obj fn weighting set
    Real iterator_start_time;
    if (iteratorCommRank == 0) {
      meta_iterator.initialize_iterator(i);
      if (messagePass)
        iterator_start_time = parallelLib.parallel_time();
    }

    // Run the iterator on the model for this iterator job
    run_iterator(sub_iterator, si_pl);

    // collect results on peer 1
    if (iteratorCommRank == 0) {
      // report iterator timings (to tagged output if concurrent iterators)
      if (messagePass) {
        Real iterator_end_time = parallelLib.parallel_time();
        Cout << "\nParameter set " << i+1 << " elapsed time = "
             << iterator_end_time - iterator_start_time << " (start: "
             << iterator_start_time << ", end: " << iterator_end_time << ")\n";
      }
      meta_iterator.update_local_results(i);
    }
  }

  // Collect results from iterator peers for output in results summary.
  // This also synchronizes the concurrent iterator servers so that the
  // ParallelLibrary destructor timings are valid (all parameter sets have
  // completed).  If no synchronization was applied, then the timings would
  // reflect only the completion of the first iterator server.
  if (iteratorServerId > 1) { // peers 2-n: send results to peer 1
    for (int i=iteratorServerId-1; i<numIteratorJobs; i+=numIteratorServers) {
      MPIPackBuffer send_buffer;//(resultsMsgLen);
      meta_iterator.pack_results_buffer(send_buffer, i);
      parallelLib.send_si(send_buffer, 0, i+1);
    }
  }
  else if (numIteratorServers > 1) { // peer 1: receive results from peers 2-n
    for (int i=1; i<numIteratorJobs; i++) { // skip 0 since this is peer 1
      int source = i%numIteratorServers;
      if (source) { // parameter set evaluated on peers 2-n
	MPI_Status status;
	MPIUnpackBuffer recv_buffer(resultsMsgLen);
	parallelLib.recv_si(recv_buffer, source, i+1, status);
	meta_iterator.unpack_results_buffer(recv_buffer, i);
      }
    }
  }
}


/** This function is similar in structure to
    ApplicationInterface::serve_evaluations_synch(). */
void IteratorScheduler::
serve_iterators(Iterator& meta_iterator, Iterator& sub_iterator)
{
  const ParallelLevel& si_pl
    = parallelLib.parallel_configuration().si_parallel_level();
  RealArray param_set;
  int job_id = 1;
  while (job_id) {

    // receive job from master
    if (iteratorCommRank == 0) {
      MPI_Status status;
      MPIUnpackBuffer recv_buffer(paramsMsgLen);
      parallelLib.recv_si(recv_buffer, 0, MPI_ANY_TAG, status);
      job_id = status.MPI_TAG;
      if (job_id)
	meta_iterator.unpack_parameters_buffer(recv_buffer);
    }
    if (iteratorCommSize > 1) // must Bcast job_id over iteratorComm
      parallelLib.bcast_i(job_id);

    if (job_id) {

      // Set starting point or obj fn weighting set
      Real iterator_start_time;
      if (iteratorCommRank == 0)
	iterator_start_time = parallelLib.parallel_time();

      // Run the iterator on the model for the received job
      run_iterator(sub_iterator, si_pl);

      if (iteratorCommRank == 0) {
	Real iterator_end_time = parallelLib.parallel_time();
	Cout << "\nParameter set " << job_id << " elapsed time = "
	     << iterator_end_time - iterator_start_time << " (start: "
	     << iterator_start_time << ", end: " << iterator_end_time <<")\n";
	int job_index = job_id - 1;
	meta_iterator.update_local_results(job_index);
        MPIPackBuffer send_buffer(resultsMsgLen);
	meta_iterator.pack_results_buffer(send_buffer, job_index);
        parallelLib.send_si(send_buffer, 0, job_id);
      }
    }
  }
}

} // namespace Dakota
