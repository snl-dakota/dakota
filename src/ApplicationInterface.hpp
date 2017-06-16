/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApplicationInterface
//- Description:  Abstract base class for the analysis code simulators used
//-               to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ApplicationInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef APPLICATION_INTERFACE_H
#define APPLICATION_INTERFACE_H

#include "DakotaInterface.hpp"
#include "PRPMultiIndex.hpp"
#include "ParallelLibrary.hpp"
#include "DataMethod.hpp"

namespace Dakota {

#define FALL_THROUGH 0
#define BLOCK 1

class ParamResponsePair;
class ActiveSet;


/// Derived class within the interface class hierarchy for supporting
/// interfaces to simulation codes.

/** ApplicationInterface provides an interface class for performing
    parameter to response mappings using simulation code(s).  It
    provides common functionality for a number of derived classes and
    contains the majority of all of the scheduling algorithms in
    DAKOTA.  The derived classes provide the specifics for managing
    code invocations using system calls, forks, direct procedure
    calls, or distributed resource facilities. */

class ApplicationInterface: public Interface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  ApplicationInterface(const ProblemDescDB& problem_db); ///< constructor
  ~ApplicationInterface();                               ///< destructor

protected:

  //
  //- Heading: Member functions
  //

  //
  //- Heading: Virtual function redefinitions
  //

  void init_communicators(const IntArray& message_lengths, 
			  int max_eval_concurrency);
  void set_communicators(const IntArray& message_lengths,
			 int max_eval_concurrency);
  //void free_communicators();

  void init_serial();

  /// return asynchLocalEvalConcurrency
  int asynch_local_evaluation_concurrency() const;

  /// return interfaceSynchronization
  short interface_synchronization() const;

  /// return evalCacheFlag
  bool evaluation_cache() const;
  /// return evalCacheFlag
  bool restart_file() const;

  // Placeholders for external layer of filtering (common I/O operations
  // such as d.v. linking and response time history smoothing)
  //void filter(const Variables& vars);
  //void filter(Response& response, const ActiveSet& set);

  //
  //- Heading: Member functions (evaluations)
  //

  /// Provides a "mapping" of variables to responses using a simulation.
  /// Protected due to Interface letter-envelope idiom.
  void map(const Variables& vars, const ActiveSet& set, Response& response,
	   bool asynch_flag = false);

  /// manages a simulation failure using abort/retry/recover/continuation
  void manage_failure(const Variables& vars, const ActiveSet& set,
		      Response& response, int failed_eval_id);

  /// executes a blocking schedule for asynchronous evaluations in the
  /// beforeSynchCorePRPQueue and returns all jobs
  const IntResponseMap& synchronize();

  /// executes a nonblocking schedule for asynchronous evaluations in the
  /// beforeSynchCorePRPQueue and returns a partial set of completed jobs
  const IntResponseMap& synchronize_nowait();

  /// run on evaluation servers to serve the iterator master
  void serve_evaluations();

  /// used by the iterator master to terminate evaluation servers
  void stop_evaluation_servers();

  /// checks on multiprocessor analysis configuration
  bool check_multiprocessor_analysis(bool warn);
  /// checks on asynchronous configuration (for direct interfaces)
  bool check_asynchronous(bool warn, int max_eval_concurrency);
  /// checks on asynchronous settings for multiprocessor partitions
  bool check_multiprocessor_asynchronous(bool warn, int max_eval_concurrency);

  //
  //- Heading: Virtual functions (evaluations)
  //

  /// Called by map() and other functions to execute the simulation
  /// in synchronous mode.  The portion of performing an evaluation
  /// that is specific to a derived class.
  virtual void derived_map(const Variables& vars, const ActiveSet& set,
			   Response& response, int fn_eval_id);

  /// Called by map() and other functions to execute the simulation
  /// in asynchronous mode.  The portion of performing an
  /// asynchronous evaluation that is specific to a derived class.
  virtual void derived_map_asynch(const ParamResponsePair& pair);

  /// For asynchronous function evaluations, this method is used to
  /// detect completion of jobs and process their results.  It
  /// provides the processing code that is specific to derived
  /// classes.  This version waits for at least one completion.
  virtual void wait_local_evaluations(PRPQueue& prp_queue);

  /// For asynchronous function evaluations, this method is used to
  /// detect completion of jobs and process their results.  It
  /// provides the processing code that is specific to derived
  /// classes.  This version is nonblocking and will return without
  /// any completions if none are immediately available.
  virtual void test_local_evaluations(PRPQueue& prp_queue);

  // clears any bookkeeping in derived classes
  //virtual void clear_bookkeeping();

  /// perform construct-time error checks on the parallel configuration
  virtual void init_communicators_checks(int max_eval_concurrency);
  /// perform run-time error checks on the parallel configuration
  virtual void set_communicators_checks(int max_eval_concurrency);

  //
  //- Heading: Member functions (analyses)
  //

  // Scheduling routines for message passing parallelism of analyses
  // within function evaluations (employed by derived classes):

  /// blocking dynamic schedule of all analyses within a function
  /// evaluation using message passing
  void master_dynamic_schedule_analyses();
  // manage asynchronous analysis jobs on local proc. (not currently
  // elevated to ApplicationInterface since only ForkApplicInterface
  // currently supports this)
  //void asynchronous_local_analyses(PRPQueue& prp_queue);
  // manage synchronous analysis jobs on local proc. (not currently
  // elevated to ApplicationInterface since only ForkApplicInterface
  // currently uses this)
  //void synchronous_local_analyses(PRPQueue& prp_queue);

  // Server routines for message passing parallelism of analyses
  // within function evaluations (employed by derived classes):

  /// serve the master analysis scheduler and manage one synchronous
  /// analysis job at a time
  void serve_analyses_synch();
  // serve the master analysis scheduler and manage multiple asynchronous
  // analysis jobs (not currently elevated to ApplicationInterface since
  // only ForkApplicInterface currently supports this)
  //void serve_analyses_asynch();

  //
  //- Heading: Virtual functions (analyses)
  //

  /// Execute a particular analysis (identified by analysis_id) synchronously
  /// on the local processor.  Used for the derived class specifics within
  /// ApplicationInterface::serve_analyses_synch().
  virtual int synchronous_local_analysis(int analysis_id);

  //
  //- Heading: Data
  //

  /// reference to the ParallelLibrary object used to manage MPI partitions for
  /// the concurrent evaluations and concurrent analyses parallelism levels
  ParallelLibrary& parallelLib;

  /// flag for suppressing output on slave processors
  bool suppressOutput;

  int evalCommSize;     ///< size of evalComm
  int evalCommRank;     ///< processor rank within evalComm
  int evalServerId;     ///< evaluation server identifier

  bool eaDedMasterFlag; ///< flag for dedicated master partitioning at ea level
  int analysisCommSize; ///< size of analysisComm
  int analysisCommRank; ///< processor rank within analysisComm
  int analysisServerId; ///< analysis server identifier
  int numAnalysisServers;     ///< current number of analysis servers

  /// flag for multiprocessor analysis partitions
  bool multiProcAnalysisFlag;

  /// flag for asynchronous local parallelism of analyses
  bool asynchLocalAnalysisFlag;

  /// limits the number of concurrent analyses in asynchronous local
  /// scheduling and specifies hybrid concurrency when message passing
  int asynchLocalAnalysisConcurrency;
  // NOTE: make private when analysis schedulers are elevated

  /// user specification for asynchronous local evaluation concurrency
  int asynchLocalEvalConcSpec;
  /// user specification for asynchronous local analysis concurrency
  int asynchLocalAnalysisConcSpec;

  /// the number of analysis drivers used for each function evaluation
  /// (from the analysis_drivers interface specification)
  int numAnalysisDrivers;

  /// the set of completed fn_eval_id's populated by wait_local_evaluations()
  /// and test_local_evaluations()
  IntSet completionSet;

  /// base message for managing failed evals; will be followed with
  /// more details in screen output
  String failureMessage;

private:

  //
  //- Heading: Member functions (evaluations)
  //

  /// checks data_pairs and beforeSynchCorePRPQueue to see if the current
  /// evaluation request has already been performed or queued
  bool duplication_detect(const Variables& vars, Response& response,
			  bool asynch_flag);

  /// initialize default ASV if needed; this is done at run time due
  /// to post-construct time Response size changes.
  void init_default_asv(size_t num_fns);

  // Scheduling routines employed by synchronize():

  /// blocking dynamic schedule of all evaluations in beforeSynchCorePRPQueue
  /// using message passing on a dedicated master partition; executes on
  /// iteratorComm master
  void master_dynamic_schedule_evaluations();
  /// blocking static schedule of all evaluations in beforeSynchCorePRPQueue
  /// using message passing on a peer partition; executes on iteratorComm master
  void peer_static_schedule_evaluations();
  /// blocking dynamic schedule of all evaluations in beforeSynchCorePRPQueue
  /// using message passing on a peer partition; executes on iteratorComm master
  void peer_dynamic_schedule_evaluations();
  /// perform all jobs in prp_queue using asynchronous approaches on
  /// the local processor
  void asynchronous_local_evaluations(PRPQueue& prp_queue);
  /// perform all jobs in prp_queue using synchronous approaches on
  /// the local processor
  void synchronous_local_evaluations(PRPQueue& prp_queue);

  // Scheduling routines employed by synchronize_nowait():

  /// execute a nonblocking dynamic schedule in a master-slave partition
  void master_dynamic_schedule_evaluations_nowait();
  /// execute a nonblocking static schedule in a peer partition
  void peer_static_schedule_evaluations_nowait();
  /// execute a nonblocking dynamic schedule in a peer partition
  void peer_dynamic_schedule_evaluations_nowait();
  /// launch new jobs in prp_queue asynchronously (if capacity is
  /// available), perform nonblocking query of all running jobs, and
  /// process any completed jobs (handles both local master- and local
  /// peer-scheduling cases)
  void asynchronous_local_evaluations_nowait(PRPQueue& prp_queue);

  // Utility routines for message passing schedulers

  /// convenience function for broadcasting an evaluation over an evalComm
  void broadcast_evaluation(const ParamResponsePair& pair);
  /// convenience function for broadcasting an evaluation over an evalComm
  void broadcast_evaluation(int fn_eval_id, const Variables& vars,
			    const ActiveSet& set);

  /// helper function for sending sendBuffers[buff_index] to server
  void send_evaluation(PRPQueueIter& prp_it, size_t buff_index, int server_id,
		       bool peer_flag);
  /// helper function for processing recvBuffers[buff_index] within scheduler
  void receive_evaluation(PRPQueueIter& prp_it, size_t buff_index,
			  int server_id, bool peer_flag);

  /// launch an asynchronous local evaluation
  void launch_asynch_local(PRPQueueIter& prp_it);
  /// process a completed asynchronous local evaluation
  void process_asynch_local(int fn_eval_id);
  /// process a completed synchronous local evaluation
  void process_synch_local(PRPQueueIter& prp_it);

  /// helper function for creating an initial active local queue by launching
  /// asynch local jobs from local_prp_queue, as limited by server capacity
  void assign_asynch_local_queue(PRPQueue& local_prp_queue,
				 PRPQueueIter& local_prp_iter);
  /// helper function for updating an active local queue by backfilling
  /// asynch local jobs from local_prp_queue, as limited by server capacity
  void assign_asynch_local_queue_nowait(PRPQueue& local_prp_queue,
					PRPQueueIter& local_prp_iter);

  /// helper function for testing active asynch local jobs and then backfilling
  size_t test_local_backfill(PRPQueue& assign_queue, PRPQueueIter& assign_iter);
  /// helper function for testing receive requests and then backfilling jobs
  size_t test_receives_backfill(PRPQueueIter& assign_iter, bool peer_flag);

  // Server routines employed by serve_evaluations():

  /// serve the evaluation message passing schedulers and perform
  /// one synchronous evaluation at a time
  void serve_evaluations_synch();
  /// serve the evaluation message passing schedulers and perform
  /// one synchronous evaluation at a time as part of the 1st peer
  void serve_evaluations_synch_peer();
  /// serve the evaluation message passing schedulers and manage
  /// multiple asynchronous evaluations
  void serve_evaluations_asynch();
  /// serve the evaluation message passing schedulers and perform
  /// multiple asynchronous evaluations as part of the 1st peer
  void serve_evaluations_asynch_peer();

  // Routines employed by init/set_communicators():

  /// convenience function for updating the local evaluation partition data
  /// following ParallelLibrary::init_evaluation_communicators().
  void set_evaluation_communicators(const IntArray& message_lengths);
  /// convenience function for updating the local analysis partition data
  /// following ParallelLibrary::init_analysis_communicators().
  void set_analysis_communicators();

  /// set concurrent evaluation configuration for serial operations
  void init_serial_evaluations();
  /// set concurrent analysis configuration for serial operations
  /// (e.g., for local executions on a dedicated master)
  void init_serial_analyses();

  // Routines employed by manage_failure():

  /// convenience function for the continuation approach in
  /// manage_failure() for finding the nearest successful "source"
  /// evaluation to the failed "target"
  const ParamResponsePair& get_source_pair(const Variables& target_vars);
  /// performs a 0th order continuation method to step from a
  /// successful "source" evaluation to the failed "target".
  /// Invoked by manage_failure() for failAction == "continuation".
  void continuation(const Variables& target_vars, const ActiveSet& set,
		    Response& response, const ParamResponsePair& source_pair,
		    int failed_eval_id);

  /// common input filtering operations, e.g. mesh movement
  void common_input_filtering(const Variables& vars);

  /// common output filtering operations, e.g. data filtering
  void common_output_filtering(Response& response);

  //
  //- Heading: Data
  //

  // Placeholders for external layer of filtering (common I/O operations
  // such as d.v. linking and response time history smoothing)
  //IOFilter commonInputFilter; // build a new class derived from IOFilter?
  //IOFilter commonOutputFilter;

  int worldSize;        ///< size of MPI_COMM_WORLD
  int worldRank;        ///< processor rank within MPI_COMM_WORLD

  int iteratorCommSize; ///< size of iteratorComm
  int iteratorCommRank; ///< processor rank within iteratorComm

  /// flag for message passing at ie scheduling level
  bool ieMessagePass;
  /// current number of evaluation servers
  int numEvalServers;
  /// user specification for number of evaluation servers
  int numEvalServersSpec;
  /// user specification for processors per analysis servers
  int procsPerEvalSpec;

  /// flag for message passing at ea scheduling level
  bool eaMessagePass;
  /// user spec for number of analysis servers
  int numAnalysisServersSpec;
  /// user specification for processors per analysis servers
  int procsPerAnalysisSpec;

  /// length of a MPIPackBuffer containing a Variables object;
  /// computed in Model::init_communicators()
  int lenVarsMessage;
  /// length of a MPIPackBuffer containing a Variables object and an
  /// ActiveSet object; computed in Model::init_communicators()
  int lenVarsActSetMessage;
  /// length of a MPIPackBuffer containing a Response object;
  /// computed in Model::init_communicators()
  int lenResponseMessage;
  /// length of a MPIPackBuffer containing a ParamResponsePair object;
  /// computed in Model::init_communicators()
  int lenPRPairMessage;

  /// user specification of evaluation scheduling algorithm:
  /// {DEFAULT,MASTER,PEER_DYNAMIC,PEER_STATIC}_SCHEDULING.  Used for manual
  /// overrides of auto-configure logic in ParallelLibrary::resolve_inputs().
  short evalScheduling;
  /// user specification of analysis scheduling algorithm:
  /// {DEFAULT,MASTER,PEER}_SCHEDULING.  Used for manual overrides of
  /// the auto-configure logic in ParallelLibrary::resolve_inputs().
  short analysisScheduling;

  /// limits the number of concurrent evaluations in asynchronous local
  /// scheduling and specifies hybrid concurrency when message passing
  int asynchLocalEvalConcurrency;

  /// whether the asynchronous local evaluations are to be performed
  /// with a static schedule (default false)
  bool asynchLocalEvalStatic;

  /// array with one bit per logical "server" indicating whether a job is
  /// currently running on the server (used for asynch local static schedules)
  BitArray localServerAssigned;

  /// interface synchronization specification: synchronous (default)
  /// or asynchronous
  short interfaceSynchronization;

  /// used by synchronize_nowait to manage header output frequency (since this
  /// function may be called many times prior to any completions)
  bool headerFlag;

  /// used to manage a user request to deactivate the active set vector
  /// control.
  ///   true  = modify the ASV each evaluation as appropriate (default);
  ///   false = ASV values are static so that the user need not check them
  ///           on each evaluation.
  bool asvControlFlag;

  /// used to manage a user request to deactivate the function evaluation
  /// cache (i.e., queries and insertions using the data_pairs cache).
  bool evalCacheFlag;
  /// flag indicating optional usage of tolerance-based duplication detection
  /// (less efficient, but helpful when experiencing restart cache misses)
  bool nearbyDuplicateDetect;
  /// tolerance value for tolerance-based duplication detection
  Real nearbyTolerance;

  /// used to manage a user request to deactivate the restart file (i.e., 
  /// insertions into write_restart).
  bool restartFileFlag;

  /// SharedResponseData of associated Response
  SharedResponseData sharedRespData;

  /// type of gradients present in associated Response
  String gradientType;

  /// type of Hessians present in associated Response
  String hessianType;

  /// IDs of analytic gradients when mixed gradients present
  IntSet gradMixedAnalyticIds;

  /// IDs of analytic gradients when mixed gradients present
  IntSet hessMixedAnalyticIds;

  /// the static ASV values used when the user has selected asvControl = off
  ShortArray defaultASV;

  // Failure capture settings:

  /// mitigation action for captured simulation failures: abort,
  /// retry, recover, or continuation
  String failAction;
  /// limit on the number of retries for the retry failAction
  int failRetryLimit;
  /// the dummy function values used for the recover failAction
  RealVector failRecoveryFnVals;

  // Bookkeeping lists/sets/maps/queues:

  /// used to bookkeep asynchronous evaluations which duplicate data_pairs
  /// evaluations.  Map key is evalIdCntr, map value is corresponding response.
  IntResponseMap historyDuplicateMap;

  /// used to bookkeep evalIdCntr, beforeSynchCorePRPQueue iterator, and
  /// response of asynchronous evaluations which duplicate queued
  /// beforeSynchCorePRPQueue evaluations
  std::map<int, std::pair<PRPQueueHIter, Response> > beforeSynchDuplicateMap;

  /// used to bookkeep vars/set/response of nonduplicate asynchronous core
  /// evaluations.  This is the queue of jobs populated by asynchronous map()
  /// that is later scheduled in synchronize() or synchronize_nowait().
  PRPQueue beforeSynchCorePRPQueue;

  /// used to bookkeep vars/set/response of asynchronous algebraic evaluations.
  /// This is the queue of algebraic jobs populated by asynchronous map()
  /// that is later evaluated in synchronize() or synchronize_nowait().
  PRPQueue beforeSynchAlgPRPQueue;

  /// used by nonblocking asynchronous local schedulers to bookkeep
  /// active local jobs
  PRPQueue asynchLocalActivePRPQueue;
  /// used by nonblocking message passing schedulers to bookkeep which
  /// jobs are running remotely
  std::map<int, IntSizetPair> msgPassRunningMap;
  /// fnEvalId reference point for preserving modulo arithmetic-based
  /// job assignment in case of peer static nonblocking schedulers
  int nowaitEvalIdRef;

  /// array of pack buffers for evaluation jobs queued to a server
  MPIPackBuffer*   sendBuffers;
  /// array of unpack buffers for evaluation jobs returned by a server
  MPIUnpackBuffer* recvBuffers;
  /// array of requests for nonblocking evaluation receives
  MPI_Request*     recvRequests;
};


/** DataInterface.cpp defaults of 0 servers are needed to distinguish an
    explicit user request for 1 server (serialization of a parallelism
    level) from no user request (use parallel auto-config).  This
    default causes problems when init_communicators() is not called
    for an interface object (e.g., static scheduling fails in
    DirectApplicInterface::derived_map() for NestedModel::optionalInterface).
    This is the reason for this function: to reset certain defaults for
    interface objects that are used serially. */
inline void ApplicationInterface::init_serial()
{ init_serial_evaluations(); init_serial_analyses(); }


inline void ApplicationInterface::init_serial_evaluations()
{
  numEvalServers = asynchLocalEvalConcurrency = 1;
  multiProcEvalFlag = false; // for *_communicators_checks()
}
// other evaluation defaults OK for serial operations


inline void ApplicationInterface::init_serial_analyses()
{
  numAnalysisServers = asynchLocalAnalysisConcurrency = 1;
  multiProcAnalysisFlag = false; // for *_communicators_checks()
}
// other analysis defaults OK for serial operations


inline int ApplicationInterface::asynch_local_evaluation_concurrency() const
{ return asynchLocalEvalConcurrency; }


inline short ApplicationInterface::interface_synchronization() const
{ return interfaceSynchronization; }


inline bool ApplicationInterface::evaluation_cache() const
{ return evalCacheFlag; }


inline bool ApplicationInterface::restart_file() const
{ return restartFileFlag; }


inline void ApplicationInterface::
derived_map(const Variables& vars, const ActiveSet& set, Response& response,
	    int fn_eval_id)
{
  Cerr << "\nError: no default definition of virtual derived_map() function "
       << "defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
}


inline void ApplicationInterface::
derived_map_asynch(const ParamResponsePair& pair)
{
  Cerr << "\nError: no default definition of virtual derived_map_asynch() "
       << "function defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
}


inline void ApplicationInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  Cerr << "\nError: no default definition of virtual wait_local_evaluations() "
       << "function defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
}


inline void ApplicationInterface::test_local_evaluations(PRPQueue& prp_queue)
{
  Cerr << "\nError: no default definition of virtual test_local_evaluations() "
       << "function defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
}


inline int ApplicationInterface::
synchronous_local_analysis(int analysis_id)
{
  Cerr << "\nError: no default definition of virtual synchronous_local_analysis"
       << "() function defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
  return 0;
}


inline void ApplicationInterface::
broadcast_evaluation(const ParamResponsePair& pair)
{ broadcast_evaluation(pair.eval_id(), pair.variables(), pair.active_set()); }


inline void ApplicationInterface::
send_evaluation(PRPQueueIter& prp_it, size_t buff_index, int server_id,
		bool peer_flag)
{
  if (sendBuffers[buff_index].size()) // reuse of existing send/recv buffers
    { sendBuffers[buff_index].reset(); recvBuffers[buff_index].reset(); }
  else {                              // freshly allocated send/recv buffers
    //sendBuffers[buff_index].resize(lenVarsActSetMessage); // protected
    recvBuffers[buff_index].resize(lenResponseMessage);
  }
  sendBuffers[buff_index] << prp_it->variables() << prp_it->active_set();

  int fn_eval_id = prp_it->eval_id();
  if (outputLevel > SILENT_OUTPUT) {
    if (peer_flag) {
      Cout << "Peer 1 assigning ";
      if (!interfaceId.empty()) Cout << interfaceId << ' ';
      Cout << "evaluation " << fn_eval_id << " to peer "
	   << server_id+1 << '\n'; // 2 to numEvalServers
    }
    else {
      Cout << "Master assigning ";
      if (!interfaceId.empty()) Cout << interfaceId << ' ';
      Cout << "evaluation " << fn_eval_id << " to server "
	   << server_id << '\n';
    }
  }

#ifdef MPI_DEBUG
  Cout << "send_evaluation() buff_index = " << buff_index << " fn_eval_id = "
       << fn_eval_id << " server_id = " << server_id << std::endl;
#endif // MPI_DEBUG

  // pre-post nonblocking receives (to prevent any message buffering)
  parallelLib.irecv_ie(recvBuffers[buff_index], server_id, fn_eval_id,
		       recvRequests[buff_index]);
  // nonblocking sends: master assigns jobs to evaluation servers
  MPI_Request send_request; // only 1 needed
  parallelLib.isend_ie(sendBuffers[buff_index], server_id, fn_eval_id,
		       send_request);
  parallelLib.free(send_request); // no test/wait on send_request
}


inline void ApplicationInterface::launch_asynch_local(PRPQueueIter& prp_it)
{
  int fn_eval_id = prp_it->eval_id();
  if (outputLevel > SILENT_OUTPUT) {
    Cout << "Initiating ";
    if (!interfaceId.empty()) Cout << interfaceId << ' ';
    Cout << "evaluation " << fn_eval_id << '\n';
  }

  // bcast job to other processors within peer 1 (added for direct plugins)
  if (multiProcEvalFlag)
    broadcast_evaluation(*prp_it);

  derived_map_asynch(*prp_it);
  asynchLocalActivePRPQueue.insert(*prp_it);
}


//inline void ApplicationInterface::clear_bookkeeping()
//{ } // virtual function: default behavior does nothing

} // namespace Dakota

#endif
