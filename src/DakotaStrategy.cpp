/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Strategy
//- Description: Implementation code for the Strategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "DakotaStrategy.hpp"
#include "CollaborativeHybridStrategy.hpp"
#include "EmbeddedHybridStrategy.hpp"
#include "SequentialHybridStrategy.hpp"
#include "ConcurrentStrategy.hpp"
#include "SingleMethodStrategy.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"


static const char rcsId[]="@(#) $Id: DakotaStrategy.cpp 6749 2010-05-03 17:11:57Z briadam $";

using namespace std;

namespace Dakota {

extern ProblemDescDB   dummy_db;  // defined in dakota_global_defs.cpp
extern ParallelLibrary dummy_lib; // defined in dakota_global_defs.cpp


/** This constructor is the one which must build the base class data for all
    inherited strategies.  get_strategy() instantiates a derived class letter
    and the derived constructor selects this base class constructor in its 
    initialization list (to avoid the recursion of the base class constructor
    calling get_strategy() again).  Since the letter IS the representation, 
    its representation pointer is set to  NULL (an uninitialized pointer 
    causes problems in ~Strategy). */
Strategy::Strategy(BaseConstructor, ProblemDescDB& problem_db): 
  probDescDB(problem_db), parallelLib(probDescDB.parallel_library()),
  strategyName(probDescDB.get_string("strategy.type")), 
  worldRank(parallelLib.world_rank()), worldSize(parallelLib.world_size()),
  graph2DFlag(probDescDB.get_bool("strategy.graphics")),
  tabularDataFlag(probDescDB.get_bool("strategy.tabular_graphics_data")),
  tabularDataFile(probDescDB.get_string("strategy.tabular_graphics_file")),
  resultsOutputFlag(probDescDB.get_bool("strategy.results_output")),
  resultsOutputFile(probDescDB.get_string("strategy.results_output_file")),
  numIteratorJobs(1), maxConcurrency(1), strategyRep(NULL), referenceCount(1)
{
  int db_write_precision = probDescDB.get_int("strategy.output_precision");
  if (db_write_precision > 0) {
    if (db_write_precision > 16) {
      std::cout << "\nWarning: requested output_precision exceeds DAKOTA's "
		<< "internal precision;\n         resetting to 16." 
		<< std::endl;
      write_precision = 16;
    }
    else
      write_precision = db_write_precision;
  }
#ifdef REFCOUNT_DEBUG
  cout << "Strategy::Strategy(BaseConstructor, ProblemDescDB) called to build "
       << "letter base class." << endl;
#endif
}


/** Default constructor.  strategyRep is NULL in this case (a
    populated problem_db is needed to build a meaningful Strategy
    object).  This makes it necessary to check for NULL in the copy
    constructor, assignment operator, and destructor. */
Strategy::Strategy(): probDescDB(dummy_db), parallelLib(dummy_lib),
  strategyRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  cout << "Strategy::Strategy() called to build empty envelope base class "
       << "object." << endl;
#endif
}


/** Used in main.cpp instantiation to build the envelope.  This constructor only
    needs to extract enough data to properly execute get_strategy, since 
    Strategy::Strategy(BaseConstructor, problem_db) builds the actual base
    class data inherited by the derived strategies. */
Strategy::Strategy(ProblemDescDB& problem_db): 
  probDescDB(problem_db), parallelLib(probDescDB.parallel_library()),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  cout << "Strategy::Strategy(ProblemDescDB&) called to instantiate envelope."
       << endl;
#endif

  // set the rep pointer to the appropriate strategy type
  strategyRep = get_strategy();
  if ( !strategyRep ) // bad name or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize strategyRep to the 
    appropriate derived type, as given by the strategyName attribute. */
Strategy* Strategy::get_strategy()
{
#ifdef REFCOUNT_DEBUG
  cout << "Envelope instantiating letter: Getting strategy " << strategyName 
       << endl;
#endif

  const String& strategy_type = probDescDB.get_string("strategy.type");
  if (strategy_type == "hybrid") {
    const String& hybrid_type = probDescDB.get_string("strategy.hybrid.type");
    if (hybrid_type == "collaborative")
      return new CollaborativeHybridStrategy(probDescDB);
    else if (hybrid_type == "embedded")
      return new EmbeddedHybridStrategy(probDescDB);
    else if (strbegins(hybrid_type, "sequential")) 
      // sequential,sequential_adaptive
      return new SequentialHybridStrategy(probDescDB);
    else {
      cerr << "Invalid hybrid strategy type: " << hybrid_type << endl;
      return NULL;
    }
  }
  else if (strategy_type == "multi_start" || strategy_type == "pareto_set")
    return new ConcurrentStrategy(probDescDB);
  else if (strategy_type == "single_method")
    return new SingleMethodStrategy(probDescDB);
  else {
    cerr << "Invalid strategy type: " << strategy_type << endl;
    return NULL;
  }
}


/** Copy constructor manages sharing of strategyRep and incrementing of
    referenceCount. */
Strategy::Strategy(const Strategy& strat):
  probDescDB(strat.problem_description_db()),
  parallelLib(probDescDB.parallel_library())
{
  // Increment new (no old to decrement)
  strategyRep = strat.strategyRep;
  if (strategyRep) // Check for an assignment of NULL
    strategyRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  cout << "Strategy::Strategy(Strategy&)" << endl;
  if (strategyRep)
    cout << "strategyRep referenceCount = " << strategyRep->referenceCount
	 << endl;
#endif
}


/** Assignment operator decrements referenceCount for old strategyRep, assigns
    new strategyRep, and increments referenceCount for new strategyRep. */
Strategy Strategy::operator=(const Strategy& strat)
{
  if (strategyRep != strat.strategyRep) { // normal case: old != new
    // Decrement old
    if (strategyRep) // Check for NULL
      if ( --strategyRep->referenceCount == 0 ) 
	delete strategyRep;
    // Assign new
    strategyRep = strat.strategyRep;
  }
  // Increment new (either case: old == new or old != new)
  if (strategyRep) // Check for NULL
    strategyRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  cout << "Strategy::operator=(Strategy&)" << endl;
  if (strategyRep)
    cout << "strategyRep referenceCount = " << strategyRep->referenceCount
	 << endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes strategyRep
    when referenceCount reaches zero. */
Strategy::~Strategy()
{ 
  // Check for NULL pointer 
  if (strategyRep) { // envelope: manage referenceCount & delete strategyRep
    --strategyRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    cout << "strategyRep referenceCount decremented to " 
	 << strategyRep->referenceCount << endl;
#endif
    if (strategyRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      cout << "deleting strategyRep" << endl;
#endif
      delete strategyRep;
    }
  }
  else if (!parallelLib.is_null()) // letter: base class destruction
    parallelLib.free_iterator_communicators(); // free the iterator partitions
}


void Strategy::run_strategy()
{
  if (strategyRep)
    strategyRep->run_strategy();
  else { // letter lacking redefinition of virtual fn.!
    cerr << "Error: Letter lacking redefinition of virtual run_strategy() "
         << "function.\n       No default defined at base class." << endl;
    abort_handler(-1);
  }
}


const Variables& Strategy::variables_results() const
{
  if (!strategyRep) { // letter lacking redefinition of virtual fn.!
    cerr << "Error: Letter lacking redefinition of virtual variables_results() "
         << "function.\n       No default defined at base class." << endl;
    abort_handler(-1);
  }
   
  return strategyRep->variables_results();
}


const Response& Strategy::response_results() const
{
  if (!strategyRep) { // letter lacking redefinition of virtual fn.!
    cerr << "Error: Letter lacking redefinition of virtual response_results() "
         << "function.\n       No default defined at base class." << endl;
    abort_handler(-1);
  }
   
  return strategyRep->response_results();
}


void Strategy::initialize_iterator(int job_index)
{
  if (strategyRep)
    strategyRep->initialize_iterator(job_index);
  //else: letter lacking redefinition of virtual fn.; default = no-op
}


/** This virtual function redefinition is executed on the dedicated master
    processor for self scheduling.  It is not used for peer partitions. */
void Strategy::pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  if (strategyRep)
    strategyRep->pack_parameters_buffer(send_buffer, job_index);
  //else: letter lacking redefinition of virtual fn.; default = no-op
}


/** This virtual function redefinition is executed on an iterator server for
    dedicated master self scheduling.  It is not used for  peer partitions. */
void Strategy::unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer)
{
  if (strategyRep)
    strategyRep->unpack_parameters_buffer(recv_buffer);
  //else: letter lacking redefinition of virtual fn.; default = no-op
}


/** This virtual function redefinition is executed either on an iterator
    server for dedicated master self scheduling or on peers 2 through n
    for static scheduling. */
void Strategy::pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  if (strategyRep)
    strategyRep->pack_results_buffer(send_buffer, job_index);
  //else: letter lacking redefinition of virtual fn.; default = no-op
}


/** This virtual function redefinition is executed on an strategy master
    (either the dedicated master processor for self scheduling or peer 1
    for static scheduling). */
void Strategy::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{
  if (strategyRep)
    strategyRep->unpack_results_buffer(recv_buffer, job_index);
  //else: letter lacking redefinition of virtual fn.; default = no-op
}


void Strategy::update_local_results(int job_index)
{
  if (strategyRep)
    strategyRep->update_local_results(job_index);
  //else: letter lacking redefinition of virtual fn.; default = no-op
}


/** This function is called from derived class constructors once
    maxConcurrency is defined but prior to instantiating Iterators
    and Models. */
void Strategy::init_iterator_parallelism()
{
  // Initialize iterator partitions after parsing but prior to output/restart.
  // The default setting for max_concurrency is the number of specified iterator
  // servers, which will yield a peer partition.
  int iterator_servers = probDescDB.get_int("strategy.iterator_servers");
  // TO DO: May want to set procs_per_iterator to a minimum size to accomodate
  // any lower level requests/overrides.  With default_config = "push_down",
  // this is not currently needed.
  int procs_per_iterator = 0;
  const String& iterator_scheduling
    = probDescDB.get_string("strategy.iterator_scheduling");
  // Default parallel config is currently "push_down" for all strategies:
  // >> parallel B&B has idle servers on initial phases.  When iterator servers
  //    are specified, the default max_concurrency yields a peer partition for
  //    use by distributed scheduling.
  // >> For multi_start/pareto_set, optimizer durations are likely to be large
  //    blocks with high variability; therefore, serialize at the source of the
  //    variability ("push_down").
  // Concurrent iterator approaches that might work well with "push_up":
  // >> OUU if the UQ portion was sampling-based with a fixed number of samples
  // >> Multi-start with non-optimization algorithms (e.g., parameter studies).
  String default_config = "push_down";

  const ParallelLevel& si_pl = parallelLib.init_iterator_communicators(
    iterator_servers, procs_per_iterator, maxConcurrency, default_config,
    iterator_scheduling);

  // retrieve the partition data
  stratIterDedMaster   = si_pl.dedicated_master_flag();
  stratIterMessagePass = si_pl.message_pass();
  iteratorCommRank     = si_pl.server_communicator_rank();
  iteratorCommSize     = si_pl.server_communicator_size();
  numIteratorServers   = si_pl.num_servers();
  iteratorServerId     = si_pl.server_id();

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
  parallelLib.manage_outputs_restart(si_pl, resultsOutputFlag,
				     resultsOutputFile);

  // *** TO DO: support for multiple concurrent iterator partitions.
  //
  // Supported examples of a single level of concurrent iterators:
  //   ConcurrentStrat (multi_start, pareto_set), BranchBndStrategy
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


/** This is a convenience function for encapsulating the allocation of
    communicators prior to running an iterator.  It does not require a
    strategyRep forward since it is only used by letter objects. */
void Strategy::init_iterator(Iterator& the_iterator, Model& the_model)
{
  // iteratorComm rank 0: Instantiate the iterator and initialize communicators.
  // Logic below based on iteratorCommSize could be more efficient with use of
  // ieCommSplitFlag, but this is not available until after Model::init_comms.
  if (iteratorCommRank == 0) {
    if (iteratorCommSize > 1)
      the_model.init_comms_bcast_flag(true);
    // only master processor needs an iterator object:
    the_iterator = probDescDB.get_iterator(the_model);
    the_model.init_communicators(the_iterator.maximum_concurrency());
    if (iteratorCommSize > 1)
      the_model.stop_configurations();
  }
  // iteratorComm ranks 1->n: match all the_model.init_communicators() calls
  // that occur on rank 0 (due both to implicit model recursions within the
  // Iterator constructors and the explicit call above).  The max iterator
  // concurrency (corresponding to the last/explicit init_communicators()
  // call above) must be stored for later use in set/free_comms.
  else {
    int last_concurrency = the_model.serve_configurations();
    the_iterator.maximum_concurrency(last_concurrency); // store for set/free
  }
}


/** This is a convenience function for encapsulating the parallel features
    (run/serve) of running an iterator.  This function omits
    allocation/deallocation of communicators to provide greater efficiency
    in those strategies which involve multiple iterator executions but only
    require communicator allocation/deallocation to be performed once.

    It does not require a strategyRep forward since it is only used by
    letter objects.  While it is currently a public function due to its 
    use in MINLPNode, this usage still involves a strategy letter object. */
void Strategy::run_iterator(Iterator& the_iterator, Model& the_model)
{
  // can't just get model reference using the_iterator.iterated_model()
  // since the_iterator is only instantiated on iterator leader procs and
  // the_model is needed on all procs.

  // set the correct ParallelConfiguration within ParallelLibrary
  //parallelLib.parallel_configuration_iterator(
  //  the_model.parallel_configuration_iterator());

  // segregate processors into run/serve
  if (iteratorCommRank == 0) { // iteratorCommRank defaults to worldRank
    //the_iterator.iterated_model(the_model); // not currently necessary
    the_iterator.run_iterator(Cout); // verbose mode
    the_model.stop_servers(); // Send the termination message to the servers
  }
  else {
    //ParConfigLIter prev_pc = parallelLib.parallel_configuration_iterator();
    the_model.set_communicators(the_iterator.maximum_concurrency());
    the_model.serve(); // runs until termination message is received
    //parallelLib.parallel_configuration_iterator(prev_pc); // reset
  }
}


/** This is a convenience function for encapsulating the deallocation
    of communicators after running an iterator.  It does not require a
    strategyRep forward since it is only used by letter objects. */
void Strategy::free_iterator(Iterator& the_iterator, Model& the_model)
{
  // set the correct ParallelConfiguration within ParallelLibrary
  //parallelLib.parallel_configuration_iterator(
  //  the_model.parallel_configuration_iterator());

  // free fn. evaluation subcommunicators if present.
  the_model.free_communicators(the_iterator.maximum_concurrency());
}


/** This implementation supports the scheduling of multiple jobs using
    a single iterator/model pair.  Additional future (overloaded)
    implementations could involve independent iterator instances. */
void Strategy::schedule_iterators(Iterator& the_iterator, Model& the_model)
{
  if (stratIterDedMaster) {
    if (worldRank == 0) // strategy master
      self_schedule_iterators(the_model);
    else // slave iterator servers
      serve_iterators(the_iterator, the_model);
  }
  else // static scheduling of iterator jobs
    static_schedule_iterators(the_iterator, the_model);
}


/** This function is adapted from
    ApplicationInterface::self_schedule_evaluations(). */
void Strategy::self_schedule_iterators(Model& the_model)
{
  // model.init_communicators() is called on the iterator servers, but not on
  // the strategy master.  Therefore, match collective communications.
  parallelLib.print_configuration(); // matches call within init_communicators()

  int i, num_sends = std::min(numIteratorServers, numIteratorJobs);
  cout << "First pass: assigning " << num_sends << " iterator jobs among " 
       << numIteratorServers << " servers\n";

  MPIPackBuffer*   send_buffers  = new MPIPackBuffer   [num_sends];
  MPIUnpackBuffer* recv_buffers  = new MPIUnpackBuffer [numIteratorJobs];
  MPI_Request      send_request; // only 1 needed since no test/wait on sends
  MPI_Request*     recv_requests = new MPI_Request     [num_sends];

  // assign 1st num_sends jobs
  for (i=0; i<num_sends; i++) {
    // pack the ith parameter set
    //send_buffers[i].resize(paramsMsgLen);
    pack_parameters_buffer(send_buffers[i], i);
    // pre-post receives
    recv_buffers[i].resize(resultsMsgLen);
    parallelLib.irecv_si(recv_buffers[i], i+1, i+1, recv_requests[i]);
    // nonblocking sends: master quickly assigns first num_sends jobs
    parallelLib.isend_si(send_buffers[i], i+1, i+1, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }

  // self-schedule remaining jobs
  if (num_sends < numIteratorJobs) {
    cout << "Second pass: self-scheduling " << numIteratorJobs-num_sends 
         << " remaining iterator jobs\n";
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
	unpack_results_buffer(recv_buffers[job_index], job_index);
        if (send_cntr < numIteratorJobs) {
          send_buffers[server_index].reset();// reuse send_buffers/recv_requests
	  pack_parameters_buffer(send_buffers[server_index], send_cntr);
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
    cout << "Waiting on all iterator jobs." << endl;
    parallelLib.waitall(numIteratorJobs, recv_requests);
    // All buffers received, now generate rawResponseArray
    for (i=0; i<numIteratorJobs; i++)
      unpack_results_buffer(recv_buffers[i], i);
  }
  // deallocate MPI & buffer arrays
  delete [] send_buffers;
  delete [] recv_buffers;
  delete [] recv_requests;

  // terminate servers
  for (i=0; i<numIteratorServers; i++) {
    MPIPackBuffer send_buffer(0); // empty buffer
    parallelLib.isend_si(send_buffer, i+1, 0, send_request); // send term
    parallelLib.free(send_request); // no test/wait on send_request
  }
}


/** This function is similar in structure to
    ApplicationInterface::serve_evaluations_synch(). */
void Strategy::serve_iterators(Iterator& the_iterator, Model& the_model)
{
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
	unpack_parameters_buffer(recv_buffer);
    }
    if (iteratorCommSize > 1) // must Bcast job_id over iteratorComm
      parallelLib.bcast_i(job_id);

    if (job_id) {

      // Set starting point or obj fn weighting set
      Real iterator_start_time;
      if (iteratorCommRank == 0)
	iterator_start_time = parallelLib.parallel_time();

      // Run the iterator on the model for the received job
      run_iterator(the_iterator, the_model);

      if (iteratorCommRank == 0) {
	Real iterator_end_time = parallelLib.parallel_time();
	Cout << "\nParameter set " << job_id << " elapsed time = "
	     << iterator_end_time - iterator_start_time << " (start: "
	     << iterator_start_time << ", end: " << iterator_end_time <<")\n";
	int job_index = job_id - 1;
	update_local_results(job_index);
        MPIPackBuffer send_buffer(resultsMsgLen);
	pack_results_buffer(send_buffer, job_index);
        parallelLib.send_si(send_buffer, 0, job_id);
      }
    }
  }
}


void Strategy::
static_schedule_iterators(Iterator& the_iterator, Model& the_model)
{
  for (int i=iteratorServerId-1; i<numIteratorJobs; i+=numIteratorServers) {

    // Set starting point or obj fn weighting set
    Real iterator_start_time;
    if (iteratorCommRank == 0) {
      initialize_iterator(i);
      if (stratIterMessagePass)
        iterator_start_time = parallelLib.parallel_time();
    }

    // Run the iterator on the model for this iterator job
    run_iterator(the_iterator, the_model);

    // collect results on peer 1
    if (iteratorCommRank == 0) {
      // report iterator timings (to tagged output if concurrent iterators)
      if (stratIterMessagePass) {
        Real iterator_end_time = parallelLib.parallel_time();
        Cout << "\nParameter set " << i+1 << " elapsed time = "
             << iterator_end_time - iterator_start_time << " (start: "
             << iterator_start_time << ", end: " << iterator_end_time << ")\n";
      }
      update_local_results(i);
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
      pack_results_buffer(send_buffer, i);
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
	unpack_results_buffer(recv_buffer, i);
      }
    }
  }
}

} // namespace Dakota
