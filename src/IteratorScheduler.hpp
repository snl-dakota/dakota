/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       IteratorScheduler
//- Description: 
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: IteratorScheduler.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef ITERATOR_SCHEDULER_H
#define ITERATOR_SCHEDULER_H

//#include "Scheduler.hpp"
#include "DataMethod.hpp"
#include "ParallelLibrary.hpp"


namespace Dakota {

class Iterator;
class Model;
class ProblemDescDB;


/// This class encapsulates scheduling operations for concurrent
/// sub-iteration within an outer level context (e.g., meta-iteration,
/// nested models).

/** In time, a Scheduler class hierarchy is envisioned, but for now,
    this class is not part of a hierarchy. */

class IteratorScheduler //: public Scheduler
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  // default constructor
  //IteratorScheduler();
  /// constructor
  IteratorScheduler(ParallelLibrary& parallel_lib, bool peer_assign_jobs,
		    int num_servers = 0, int procs_per_iterator = 0,
		    short scheduling = DEFAULT_SCHEDULING);
  /// destructor
  ~IteratorScheduler();
    
  //
  //- Heading: Static member functions
  //

  // convenience function for serializing the concurrent iterator
  // parallelism level
  //static void init_serial_iterators(ParallelLibrary& parallel_lib);

  /// convenience function for allocation of an iterator and (parallel)
  /// initialization of its comms
  static void init_iterator(ProblemDescDB& problem_db, Iterator& sub_iterator,
			    ParLevLIter pl_iter);
  /// convenience function for allocation of an iterator and (parallel)
  /// initialization of its comms
  static void init_iterator(ProblemDescDB& problem_db, Iterator& sub_iterator,
			    Model& sub_model, ParLevLIter pl_iter);
  /// convenience function for lightweight allocation of an iterator
  /// and (parallel) initialization of its comms
  static void init_iterator(ProblemDescDB& problem_db,
			    const String& method_string, Iterator& sub_iterator,
			    Model& sub_model, ParLevLIter pl_iter);

  /// convenience function for setting comms prior to running an iterator
  static void set_iterator(Iterator& sub_iterator, ParLevLIter pl_iter);

  /// Convenience function for invoking an iterator and managing parallelism.
  /// This version omits communicator repartitioning. Function must be public
  /// due to use by MINLPNode.
  static void run_iterator(Iterator& sub_iterator, ParLevLIter pl_iter);

  /// convenience function for deallocating comms after running an iterator
  static void free_iterator(Iterator& sub_iterator, ParLevLIter pl_iter);

  //
  //- Heading: Member functions
  //

  /// instantiate sub_iterator on the current rank if not already constructed
  void construct_sub_iterator(ProblemDescDB& problem_db, Iterator& sub_iterator,
			      Model& sub_model, const String& method_ptr,
			      const String& method_name,
			      const String& model_ptr);

  /// performs sufficient initialization to define partitioning controls
  /// (min and max processors per iterator server)
  IntIntPair configure(ProblemDescDB& problem_db, Iterator& sub_iterator,
		       Model& sub_model);
  /// performs sufficient initialization to define partitioning controls
  /// (min and max processors per iterator server)
  IntIntPair configure(ProblemDescDB& problem_db, const String& method_string,
		       Iterator& sub_iterator, Model& sub_model);
  /// performs sufficient initialization to define partitioning controls
  /// (min and max processors per iterator server)
  IntIntPair configure(ProblemDescDB& problem_db, Iterator& sub_iterator);

  /// convenience function for initializing iterator communicators, setting
  /// parallel configuration attributes, and managing outputs and restart.
  void partition(int max_iterator_concurrency, IntIntPair& ppi_pr);

  /// invokes static version of this function with appropriate parallelism level
  void init_iterator(ProblemDescDB& problem_db, Iterator& sub_iterator,
		     Model& sub_model);
  /// invokes static version of this function with appropriate parallelism level
  void init_iterator(ProblemDescDB& problem_db, const String& method_string,
		     Iterator& sub_iterator, Model& sub_model);
  /// invokes static version of this function with appropriate parallelism level
  void set_iterator(Iterator& sub_iterator);
  /// invokes static version of this function with appropriate parallelism level
  void run_iterator(Iterator& sub_iterator);
  /// invokes static version of this function with appropriate parallelism level
  void free_iterator(Iterator& sub_iterator);

  /// convenience function for deallocating the concurrent iterator
  /// parallelism level
  void free_iterator_parallelism();

  /// short convenience function for distributing control among
  /// master_dynamic_schedule_iterators(), serve_iterators(), and
  /// peer_static_schedule_iterators()
  template <typename MetaType>
  void schedule_iterators(MetaType& meta_object, Iterator& sub_iterator);
  /// executed by the scheduler master to manage a dynamic schedule of
  /// iterator jobs among slave iterator servers
  template <typename MetaType>
  void master_dynamic_schedule_iterators(MetaType& meta_object);
  /// executed by the scheduler master to terminate slave iterator servers
  void stop_iterator_servers();
  /// executed on the slave iterator servers to perform iterator jobs
  /// assigned by the scheduler master
  template <typename MetaType>
  void serve_iterators(MetaType& meta_object, Iterator& sub_iterator);
  /// executed on iterator peers to manage a static schedule of iterator jobs
  template <typename MetaType>
  void peer_static_schedule_iterators(MetaType& meta_object,
				      Iterator& sub_iterator);

  /// update schedPCIter
  void update(ParConfigLIter pc_iter);
  /// update miPLIndex as well as associated settings for
  /// concurrent iterator scheduling from the corresponding ParallelLevel
  void update(size_t index);
  /// invoke update(ParConfigLIter) and update(size_t) in sequence
  void update(ParConfigLIter pc_iter, size_t index);

  /// update paramsMsgLen and resultsMsgLen
  void iterator_message_lengths(int params_msg_len, int results_msg_len);

  /// determines if current processor is rank 0 of the parent comm
  bool lead_rank() const;

  //
  //- Heading: Data members
  //

  ParallelLibrary& parallelLib; ///< reference to the ParallelLibrary instance

  int   numIteratorJobs;    ///< number of iterator executions to schedule
  int   numIteratorServers; ///< number of concurrent iterator partitions
  int   procsPerIterator;   ///< partition size request
  //int minProcsPerIterator; //  lower bound on iterator partition size
  //int maxProcsPerIterator; //  upper bound on iterator partition size
  int   iteratorCommRank;   ///< processor rank in iteratorComm
  int   iteratorCommSize;   ///< number of processors in iteratorComm
  int   iteratorServerId;   ///< identifier for an iterator server

  bool  messagePass;        ///< flag for message passing among iterator servers
  short iteratorScheduling; ///< {DEFAULT,MASTER,PEER}_SCHEDULING
  //int maxIteratorConcurrency; // max concurrency possible in meta-algorithm
  bool peerAssignJobs;      ///< flag indicating need for peer 1 to assign jobs
                            ///< to peers 2-n

  ParConfigLIter schedPCIter; ///< iterator for active parallel configuration
  size_t miPLIndex;         ///< index of active parallel level (corresponding
                            ///< to ParallelConfiguration::miPLIters) to use
                            ///< for parallelLib send/recv

private:

  //
  //- Heading: Convenience member functions
  //

  //
  //- Heading: Data members
  //

  int  paramsMsgLen; ///< length of MPI buffer for parameter input instance(s)
  int resultsMsgLen; ///< length of MPI buffer for results  output instance(s)
};


//inline IteratorScheduler::IteratorScheduler():
//  numIteratorServers(1), numIteratorJobs(1), maxIteratorConcurrency(1)
//{ }


inline IteratorScheduler::~IteratorScheduler()
{ }


inline void IteratorScheduler::
init_iterator(ProblemDescDB& problem_db, Iterator& sub_iterator,
	      Model& sub_model)
{
  ParLevLIter pl_iter = schedPCIter->mi_parallel_level_iterator(miPLIndex);
  // if dedicated master overload, no iterator jobs can run on master, so no
  // init/set/free --> need to match init_comms() on iterator servers
  if (pl_iter->dedicated_master() && pl_iter->processors_per_server() > 1 &&
      pl_iter->server_id() == 0) {
    parallelLib.parallel_configuration_iterator(schedPCIter);
    parallelLib.print_configuration(); // match init_comms() on iterator servers
  }
  else
    init_iterator(problem_db, sub_iterator, sub_model, pl_iter);
}


inline void IteratorScheduler::
init_iterator(ProblemDescDB& problem_db, const String& method_string,
	      Iterator& sub_iterator, Model& sub_model)
{
  ParLevLIter pl_iter = schedPCIter->mi_parallel_level_iterator(miPLIndex);
  // if dedicated master overload, no iterator jobs can run on master, so no
  // init/set/free --> need to match init_comms() on iterator servers
  if (pl_iter->dedicated_master() && pl_iter->processors_per_server() > 1 &&
      pl_iter->server_id() == 0) {
    parallelLib.parallel_configuration_iterator(schedPCIter);
    parallelLib.print_configuration();
  }
  else
    init_iterator(problem_db, method_string, sub_iterator, sub_model, pl_iter);
}


inline void IteratorScheduler::set_iterator(Iterator& sub_iterator)
{
  set_iterator(sub_iterator,
	       schedPCIter->mi_parallel_level_iterator(miPLIndex));
}


inline void IteratorScheduler::run_iterator(Iterator& sub_iterator)
{
  run_iterator(sub_iterator,
	       schedPCIter->mi_parallel_level_iterator(miPLIndex));
}


inline void IteratorScheduler::free_iterator(Iterator& sub_iterator)
{
  free_iterator(sub_iterator,
		schedPCIter->mi_parallel_level_iterator(miPLIndex));
}


inline void IteratorScheduler::update(ParConfigLIter pc_iter)
{ schedPCIter = pc_iter; }


inline void IteratorScheduler::update(size_t index)
{
  // Note: update(ParConfigLIter) must precede this update for access to mi_pl

  miPLIndex = index;
  const ParallelLevel& mi_pl = schedPCIter->mi_parallel_level(index);

  // retrieve the partition data
  //dedicatedMaster = mi_pl.dedicated_master();
  messagePass        = mi_pl.message_pass();
  iteratorCommRank   = mi_pl.server_communicator_rank();
  iteratorCommSize   = mi_pl.server_communicator_size();
  iteratorServerId   = mi_pl.server_id();

  // update requests with actual
  numIteratorServers = mi_pl.num_servers();
  iteratorScheduling = (mi_pl.dedicated_master())
                     ? MASTER_SCHEDULING : PEER_SCHEDULING;
}


inline void IteratorScheduler::update(ParConfigLIter pc_iter, size_t index)
{ update(pc_iter); update(index); }


/** This implementation supports the scheduling of multiple jobs using
    a single iterator/model pair.  Additional future (overloaded)
    implementations could involve independent iterator instances. */
template <typename MetaType> void IteratorScheduler::
schedule_iterators(MetaType& meta_object, Iterator& sub_iterator)
{
  // As for invocations of an Interface (see SimulationModel.hpp and
  // NestedModel.[ch]pp), we wrap job scheduling with store/set/restore
  ParConfigLIter curr_pc_iter = parallelLib.parallel_configuration_iterator();
  parallelLib.parallel_configuration_iterator(
    meta_object.parallel_configuration_iterator());

  if (iteratorScheduling == MASTER_SCHEDULING) { //(dedicatedMaster) {
    if (lead_rank()) { // strategy master
      master_dynamic_schedule_iterators(meta_object);
      stop_iterator_servers();
    }
    else // slave iterator servers
      serve_iterators(meta_object, sub_iterator);
  }
  else { // static scheduling of iterator jobs
    if (iteratorServerId <= numIteratorServers) {
      // jobs are not assigned by messages: stop_iterator_servers() is only
      // required for an idle server partition
      peer_static_schedule_iterators(meta_object, sub_iterator);
      if (lead_rank())
	stop_iterator_servers(); // stop an idle server partition, if present
    }
    else // for occupying an idle server partition (no jobs)
      serve_iterators(meta_object, sub_iterator);
  }

  parallelLib.parallel_configuration_iterator(curr_pc_iter); // restore
}


/** This function is adapted from
    ApplicationInterface::master_dynamic_schedule_evaluations(). */
template <typename MetaType> void IteratorScheduler::
master_dynamic_schedule_iterators(MetaType& meta_object)
{
  int i, j, num_sends = std::min(numIteratorServers, numIteratorJobs);
  Cout << "Master dynamic schedule: first pass assigning " << num_sends
       << " iterator jobs among " << numIteratorServers << " servers\n";

  MPIPackBuffer*   send_buffers  = new MPIPackBuffer   [num_sends];
  MPIUnpackBuffer* recv_buffers  = new MPIUnpackBuffer [numIteratorJobs];
  MPI_Request      send_request; // only 1 needed since no test/wait on sends
  MPI_Request*     recv_requests = new MPI_Request     [num_sends];

  // assign 1st num_sends jobs
  for (i=0, j=1; i<num_sends; ++i, ++j) {
    // pack the ith parameter set
    //send_buffers[i].resize(paramsMsgLen);
    meta_object.pack_parameters_buffer(send_buffers[i], i);
    // pre-post receives
    recv_buffers[i].resize(resultsMsgLen);
    parallelLib.irecv_mi(recv_buffers[i], j, j, recv_requests[i], miPLIndex);
    // nonblocking sends: master quickly assigns first num_sends jobs
    parallelLib.isend_mi(send_buffers[i], j, j, send_request, miPLIndex);
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
      for (i=0; i<out_count; ++i) {
        int server_index = index_array[i]; // index of completed recv_request
        int server_id    = server_index + 1;        // 1 to numIteratorServers
        int job_id       = status_array[i].MPI_TAG; // 1 to numIteratorJobs
        int job_index    = job_id - 1;              // 0 to numIteratorJobs-1
	meta_object.unpack_results_buffer(recv_buffers[job_index], job_index);
        if (send_cntr < numIteratorJobs) {
          send_buffers[server_index].reset();// reuse send_buffers/recv_requests
	  meta_object.pack_parameters_buffer(send_buffers[server_index],
					     send_cntr);
	  // pre-post receive
          recv_buffers[send_cntr].resize(resultsMsgLen);
          parallelLib.irecv_mi(recv_buffers[send_cntr], server_id, send_cntr+1, 
                               recv_requests[server_index], miPLIndex);
          // send next job to open server
          parallelLib.isend_mi(send_buffers[server_index], server_id,
                               send_cntr+1, send_request, miPLIndex);
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
    for (i=0; i<numIteratorJobs; ++i)
      meta_object.unpack_results_buffer(recv_buffers[i], i);
  }
  // deallocate MPI & buffer arrays
  delete [] send_buffers;
  delete [] recv_buffers;
  delete [] recv_requests;
}


template <typename MetaType> void IteratorScheduler::
peer_static_schedule_iterators(MetaType& meta_object, Iterator& sub_iterator)
{
  bool rank0 = (iteratorCommRank == 0); int i;

  // Some clients require job assignment, whereas other define jobs from a
  // shared specification (or a combination in case of spec + random jobs)
  if (peerAssignJobs) {
    if (rank0) {
      // transmit number of jobs to all peer leaders
      parallelLib.bcast_mi(numIteratorJobs, miPLIndex);
      // Assign jobs 
      if (iteratorServerId > 1) { // peers 2-n: recv job from peer 1
	for (i=iteratorServerId-1; i<numIteratorJobs; i+=numIteratorServers) {
	  MPI_Status status;
	  MPIUnpackBuffer recv_buffer(paramsMsgLen);
	  parallelLib.recv_mi(recv_buffer, 0, i+1, status, miPLIndex);
	  meta_object.unpack_parameters_buffer(recv_buffer, i);
	}
      }
      else if (numIteratorServers > 1) { // peer 1: recv results from peers 2-n
	for (i=1; i<numIteratorJobs; ++i) { // skip 0 since this is peer 1
	  int dest = i%numIteratorServers;
	  if (dest) { // parameter set assigned to peers 2-n
	    MPIPackBuffer send_buffer;//(paramsMsgLen);
	    meta_object.pack_parameters_buffer(send_buffer, i);
	    parallelLib.send_mi(send_buffer, dest, i+1, miPLIndex);
	  }
	}
      }
    }
    // now transmit number of jobs from iterator comm leaders to other ranks
    // (needed for proper execution of loop by all ranks)
    if (iteratorCommSize > 1)
      parallelLib.bcast_i(numIteratorJobs, miPLIndex);
  }

  // Execute the iterator jobs using round robin assignment
  for (i=iteratorServerId-1; i<numIteratorJobs; i+=numIteratorServers) {

    // Set starting point or obj fn weighting set
    Real iterator_start_time;
    if (rank0) {
      meta_object.initialize_iterator(i);
      if (messagePass)
        iterator_start_time = parallelLib.parallel_time();
    }

    // Run the iterator on the model for this iterator job
    run_iterator(sub_iterator);

    // collect results on peer 1
    if (rank0) {
      // report iterator timings (to tagged output if concurrent iterators)
      if (messagePass) {
        Real iterator_end_time = parallelLib.parallel_time();
        Cout << "\nParameter set " << i+1 << " elapsed time = "
             << iterator_end_time - iterator_start_time << " (start: "
             << iterator_start_time << ", end: " << iterator_end_time << ")\n";
      }
      meta_object.update_local_results(i);
    }
  }

  // Collect results from iterator peers for output in results summary.
  // This also synchronizes the concurrent iterator servers so that the
  // ParallelLibrary destructor timings are valid (all parameter sets have
  // completed).  If no synchronization was applied, then the timings would
  // reflect only the completion of the first iterator server.
  if (rank0) {
    if (iteratorServerId > 1) { // peers 2-n: send results to peer 1
      for (i=iteratorServerId-1; i<numIteratorJobs; i+=numIteratorServers) {
	MPIPackBuffer send_buffer;//(resultsMsgLen);
	meta_object.pack_results_buffer(send_buffer, i);
	parallelLib.send_mi(send_buffer, 0, i+1, miPLIndex);
      }
    }
    else if (numIteratorServers > 1) { // peer 1: receive results from peers 2-n
      for (i=1; i<numIteratorJobs; ++i) { // skip 0 since this is peer 1
	int source = i%numIteratorServers;
	if (source) { // parameter set evaluated on peers 2-n
	  MPI_Status status;
	  MPIUnpackBuffer recv_buffer(resultsMsgLen);
	  parallelLib.recv_mi(recv_buffer, source, i+1, status, miPLIndex);
	  meta_object.unpack_results_buffer(recv_buffer, i);
	}
      }
    }
  }
}


/** This function is similar in structure to
    ApplicationInterface::serve_evaluations_synch(). */
template <typename MetaType> void IteratorScheduler::
serve_iterators(MetaType& meta_object, Iterator& sub_iterator)
{
  RealArray param_set;
  int job_id = 1;
  while (job_id) {

    // receive job from master
    if (iteratorCommRank == 0) {
      MPI_Status status;
      MPIUnpackBuffer recv_buffer(paramsMsgLen);
      parallelLib.recv_mi(recv_buffer, 0, MPI_ANY_TAG, status, miPLIndex);
      job_id = status.MPI_TAG;
      if (job_id)
	meta_object.unpack_parameters_initialize(recv_buffer, job_id-1);
    }
    if (iteratorCommSize > 1) // must Bcast job_id over iteratorComm
      parallelLib.bcast_i(job_id, miPLIndex);

    if (job_id) {

      // Set starting point or obj fn weighting set
      Real iterator_start_time;
      if (iteratorCommRank == 0)
	iterator_start_time = parallelLib.parallel_time();

      // Run the iterator on the model for the received job
      run_iterator(sub_iterator);

      if (iteratorCommRank == 0) {
	Real iterator_end_time = parallelLib.parallel_time();
	Cout << "\nParameter set " << job_id << " elapsed time = "
	     << iterator_end_time - iterator_start_time << " (start: "
	     << iterator_start_time << ", end: " << iterator_end_time <<")\n";
	int job_index = job_id - 1;
	meta_object.update_local_results(job_index);
        MPIPackBuffer send_buffer(resultsMsgLen);
	meta_object.pack_results_buffer(send_buffer, job_index);
        parallelLib.send_mi(send_buffer, 0, job_id, miPLIndex);
      }
    }
  }
}


inline void IteratorScheduler::
iterator_message_lengths(int params_msg_len, int results_msg_len)
{ paramsMsgLen = params_msg_len; resultsMsgLen = results_msg_len; }


inline bool IteratorScheduler::lead_rank() const
{
  return ( iteratorCommRank == 0 && ( !messagePass ||
    ( iteratorScheduling == MASTER_SCHEDULING && iteratorServerId == 0 ) ||
    ( iteratorScheduling == PEER_SCHEDULING   && iteratorServerId == 1 ) ) );
}

} // namespace Dakota

#endif
