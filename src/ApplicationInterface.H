/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApplicationInterface
//- Description:  Abstract base class for the analysis code simulators used
//-               to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ApplicationInterface.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef APPLICATION_INTERFACE_H
#define APPLICATION_INTERFACE_H

#include "DakotaInterface.H"
#include "PRPMultiIndex.H"

namespace Dakota {

#define FALL_THROUGH 0
#define BLOCK 1

class ParamResponsePair;
class ParallelLibrary;
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
			  const int& max_iterator_concurrency);
  void set_communicators(const IntArray& message_lengths);
  void free_communicators();

  void init_serial();

  /// return asynchLocalEvalConcurrency
  int asynch_local_evaluation_concurrency() const;

  /// return interfaceSynchronization
  String interface_synchronization() const;

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
	   const bool asynch_flag = false);

  /// manages a simulation failure using abort/retry/recover/continuation
  void manage_failure(const Variables& vars, const ActiveSet& set,
		      Response& response, int failed_eval_id);

  /// executes a blocking schedule for asynchronous evaluations in the
  /// beforeSynchCorePRPQueue and returns all jobs
  const IntResponseMap& synch();

  /// executes a nonblocking schedule for asynchronous evaluations in the
  /// beforeSynchCorePRPQueue and returns a partial set of completed jobs
  const IntResponseMap& synch_nowait();

  /// run on evaluation servers to serve the iterator master
  void serve_evaluations();

  /// used by the iterator master to terminate evaluation servers
  void stop_evaluation_servers();

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
  virtual void derived_synch(PRPQueue& prp_queue);

  /// For asynchronous function evaluations, this method is used to
  /// detect completion of jobs and process their results.  It
  /// provides the processing code that is specific to derived
  /// classes.  This version is nonblocking and will return without
  /// any completions if none are immediately available.
  virtual void derived_synch_nowait(PRPQueue& prp_queue);

  // clears any bookkeeping in derived classes
  //virtual void clear_bookkeeping();

  //
  //- Heading: Member functions (analyses)
  //

  // Scheduling routines for message passing parallelism of analyses
  // within function evaluations (employed by derived classes):

  /// blocking self-schedule of all analyses within a function
  /// evaluation using message passing
  void self_schedule_analyses();
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
  virtual int derived_synchronous_local_analysis(const int& analysis_id);

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
  int numAnalysisServers; ///< number of analysis servers

  /// flag for multiprocessor analysis partitions
  bool multiProcAnalysisFlag;

  /// flag for asynchronous local parallelism of analyses
  bool asynchLocalAnalysisFlag;

  /// limits the number of concurrent analyses in asynchronous local
  /// scheduling and specifies hybrid concurrency when message passing
  int asynchLocalAnalysisConcurrency;
  // NOTE: make private when analysis schedulers are elevated

  /// the number of analysis drivers used for each function evaluation
  /// (from the analysis_drivers interface specification)
  int numAnalysisDrivers;

  /// the set of completed fn_eval_id's populated by derived_synch()
  /// and derived_synch_nowait()
  IntSet completionSet;

private:

  //
  //- Heading: Member functions (evaluations)
  //

  /// checks data_pairs and beforeSynchCorePRPQueue to see if the current
  /// evaluation request has already been performed or queued
  bool duplication_detect(const Variables& vars, Response& response,
			  const bool asynch_flag);

  // Scheduling routines employed by synch():

  /// blocking self-schedule of all evaluations in beforeSynchCorePRPQueue
  /// using message passing; executes on iteratorComm master
  void self_schedule_evaluations();
  /// blocking static schedule of all evaluations in beforeSynchCorePRPQueue
  /// using message passing; executes on iteratorComm master
  void static_schedule_evaluations();
  /// perform all jobs in prp_queue using asynchronous approaches on
  /// the local processor
  void asynchronous_local_evaluations(PRPQueue& prp_queue);
  /// perform all the jobs in prp_queue using asynchronous approaches
  /// on the local processor, but schedule statically such that
  /// eval_id is always replaced with an equivalent one, modulo
  /// asynchLocalEvalConcurrency
  void asynchronous_local_evaluations_static(PRPQueue& prp_queue);
  /// perform all jobs in prp_queue using synchronous approaches on
  /// the local processor
  void synchronous_local_evaluations(PRPQueue& prp_queue);

  // Scheduling routines employed by synch_nowait():

  //void self_schedule_evals_message_passing_nowait();
  //void static_schedule_evals_message_passing_nowait();
  /// launch new jobs in prp_queue asynchronously (if capacity is
  /// available), perform nonblocking query of all running jobs, and
  /// process any completed jobs (handles both local self- and local
  /// static-scheduling cases)
  void asynchronous_local_evaluations_nowait(PRPQueue& prp_queue);

  // Server routines employed by serve_evaluations():

  /// serve the evaluation message passing schedulers and perform
  /// one synchronous evaluation at a time
  void serve_evaluations_synch();
  /// serve the evaluation message passing schedulers and manage
  /// multiple asynchronous evaluations
  void serve_evaluations_asynch();
  /// serve the evaluation message passing schedulers and perform
  /// one synchronous evaluation at a time as part of the 1st peer
  void serve_evaluations_peer();

  // Routines employed by init/set_communicators():

  /// convenience function for updating the local evaluation partition data
  /// following ParallelLibrary::init_evaluation_communicators().
  void set_evaluation_communicators(const IntArray& message_lengths);
  /// convenience function for updating the local analysis partition data
  /// following ParallelLibrary::init_analysis_communicators().
  void set_analysis_communicators();
  /// perform some error checks on the parallel configuration
  void check_configuration(const int& max_iterator_concurrency);

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

  bool ieMessagePass;   ///< flag for message passing at ie scheduling level
  int numEvalServers;   ///< number of evaluation servers

  bool eaMessagePass;   ///< flag for message passing at ea scheduling level
  int procsPerAnalysis; ///< processors per analysis servers

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

  /// user specification of evaluation scheduling algorithm (self,
  /// static, or no spec).  Used for manual overrides of the
  /// auto-configure logic in ParallelLibrary::resolve_inputs().
  String evalScheduling;
  /// user specification of analysis scheduling algorithm (self,
  /// static, or no spec).  Used for manual overrides of the
  /// auto-configure logic in ParallelLibrary::resolve_inputs().
  String analysisScheduling;

  /// limits the number of concurrent evaluations in asynchronous local
  /// scheduling and specifies hybrid concurrency when message passing
  int asynchLocalEvalConcurrency;

  /// whether the asynchronous local evaluations are to be performed
  /// with a static schedule (default false)
  bool asynchLocalEvalStatic;

  /// array with one entry per local "server" indicating the job
  /// (fn_eval_id) currently running on the server (used for
  /// asynchronour local static schedules)
  IntArray localServerJobMap;

  /// interface synchronization specification: synchronous (default)
  /// or asynchronous
  String interfaceSynchronization;

  /// used by synch_nowait to manage output frequency (since this
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

  /// used to manage a user request to deactivate the restart file (i.e., 
  /// insertions into write_restart).
  bool restartFileFlag;

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
  /// evaluations.  Map key is fnEvalId, map value is corresponding response.
  IntResponseMap historyDuplicateMap;

  /// used to bookkeep fnEvalId, beforeSynchCorePRPQueue iterator, and
  /// response of asynchronous evaluations which duplicate queued
  /// beforeSynchCorePRPQueue evaluations
  std::map<int, std::pair<PRPQueueHIter, Response> > beforeSynchDuplicateMap;

  /// used to bookkeep vars/set/response of nonduplicate asynchronous core
  /// evaluations.  This is the queue of jobs populated by asynchronous map()
  /// that is later scheduled in synch() or synch_nowait().
  PRPQueue beforeSynchCorePRPQueue;

  /// used to bookkeep vars/set/response of asynchronous algebraic evaluations.
  /// This is the queue of algebraic jobs populated by asynchronous map()
  /// that is later evaluated in synch() or synch_nowait().
  PRPQueue beforeSynchAlgPRPQueue;

  /// used by asynchronous_local_nowait to bookkeep which jobs are running
  IntSet runningSet;
};


/** DataInterface.C defaults of 0 servers are needed to distinguish an
    explicit user request for 1 server (serialization of a parallelism
    level) from no user request (use parallel auto-config).  This
    default causes problems when init_communicators() is not called
    for an interface object (e.g., static scheduling fails in
    DirectApplicInterface::derived_map() for NestedModel::optionalInterface).
    This is the reason for this function: to reset certain defaults for
    interface objects that are used serially. */
inline void ApplicationInterface::init_serial()
{
  // defaults of all other attributes are OK for serial operations
  numEvalServers = numAnalysisServers = 1;
}


inline int ApplicationInterface::asynch_local_evaluation_concurrency() const
{ return asynchLocalEvalConcurrency; }


inline String ApplicationInterface::interface_synchronization() const
{ return interfaceSynchronization; }


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


inline void ApplicationInterface::derived_synch(PRPQueue& prp_queue)
{
  Cerr << "\nError: no default definition of virtual derived_synch() function "
       << "defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
}


inline void ApplicationInterface::derived_synch_nowait(PRPQueue& prp_queue)
{
  Cerr << "\nError: no default definition of virtual derived_synch_nowait() "
       << "function defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
}


inline int ApplicationInterface::
derived_synchronous_local_analysis(const int& analysis_id)
{
  Cerr << "\nError: no default definition of virtual derived_synchronous_local_"
       << "analysis() function defined in ApplicationInterface\n." << std::endl;
  abort_handler(-1);
  return 0;
}


//inline void ApplicationInterface::clear_bookkeeping()
//{ } // virtual function: default behavior does nothing

} // namespace Dakota

#endif
