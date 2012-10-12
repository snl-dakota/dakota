/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApplicationInterface
//- Description:  Implementation of base class for application interfaces
//- Owner:        Mike Eldred

#include "system_defs.h"
#include "DakotaBinStream.H"
#include "ApplicationInterface.H"
#include "ParamResponsePair.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"

namespace Dakota {
  extern BoStream write_restart;
  // globals: cache of param-response pairs & binary restart log
  extern PRPCache data_pairs; // global container

ApplicationInterface::
ApplicationInterface(const ProblemDescDB& problem_db):
  Interface(BaseConstructor(), problem_db),
  parallelLib(problem_db.parallel_library()), suppressOutput(false),
  evalCommSize(1), evalCommRank(0), evalServerId(1), eaDedMasterFlag(false),
  analysisCommSize(1), analysisCommRank(0), analysisServerId(1),
  numAnalysisServers(problem_db.get_int("interface.analysis_servers")),
  multiProcAnalysisFlag(false), asynchLocalAnalysisFlag(false),
  asynchLocalAnalysisConcurrency(
    problem_db.get_int("interface.asynch_local_analysis_concurrency")),
  numAnalysisDrivers(
    problem_db.get_sa("interface.application.analysis_drivers").size()),
  worldSize(parallelLib.world_size()), worldRank(parallelLib.world_rank()),
  iteratorCommSize(1), iteratorCommRank(0), ieMessagePass(false),
  numEvalServers(problem_db.get_int("interface.evaluation_servers")),
  eaMessagePass(false), 
  procsPerAnalysis(
    problem_db.get_int("interface.direct.processors_per_analysis")),
  lenVarsMessage(0), lenVarsActSetMessage(0), lenResponseMessage(0),
  lenPRPairMessage(0),
  evalScheduling(problem_db.get_string("interface.evaluation_scheduling")),
  analysisScheduling(problem_db.get_string("interface.analysis_scheduling")),
  asynchLocalEvalConcurrency(
    problem_db.get_int("interface.asynch_local_evaluation_concurrency")),
  asynchLocalEvalStatic(
    problem_db.get_string("interface.local_evaluation_scheduling") == "static"),
  interfaceSynchronization(problem_db.get_string("interface.synchronization")),
  headerFlag(true),
  asvControlFlag(problem_db.get_bool("interface.active_set_vector")),
  evalCacheFlag(problem_db.get_bool("interface.evaluation_cache")),
  restartFileFlag(problem_db.get_bool("interface.restart_file")),
  failAction(problem_db.get_string("interface.failure_capture.action")),
  failRetryLimit(problem_db.get_int("interface.failure_capture.retry_limit")),
  failRecoveryFnVals(
    problem_db.get_rv("interface.failure_capture.recovery_fn_vals"))
{
  // set coreMappings flag based on presence of analysis_drivers specification
  coreMappings = (numAnalysisDrivers > 0);
  if (!coreMappings && !algebraicMappings) {
    Cerr << "\nError: no parameter to response mapping defined in "
	 << "ApplicationInterface.\n" << std::endl;
    abort_handler(-1);
  }

  // If the user has specified active_set_vector as off, then map() uses a
  // default ASV which is constant for all function evaluations (so that the
  // user need not check the content of the ASV on each evaluation).
  if (!asvControlFlag) {
    size_t num_fns = (outputLevel > NORMAL_OUTPUT) ? fnLabels.size() :
      problem_db.get_sa("responses.labels").size();
    defaultASV.resize(num_fns);
    short asv_value = 1;
    const std::string& grad_type
      = problem_db.get_string("responses.gradient_type");
    const std::string& hess_type
      = problem_db.get_string("responses.hessian_type");
    if (grad_type == "analytic")
      asv_value += 2;
    if (hess_type == "analytic")
      asv_value += 4;
    defaultASV.assign(defaultASV.size(), asv_value);
    if (grad_type == "mixed") {
      const IntList& id_anal_grad
	= problem_db.get_il("responses.gradients.mixed.id_analytic");
      for (ILCIter cit=id_anal_grad.begin(); cit!=id_anal_grad.end(); ++cit)
        defaultASV[*cit - 1] += 2;
    }
    if (hess_type == "mixed") {
      const IntList& id_anal_hess
	= problem_db.get_il("responses.hessians.mixed.id_analytic");
      for (ILCIter cit=id_anal_hess.begin(); cit!=id_anal_hess.end(); ++cit)
        defaultASV[*cit - 1] += 4;
    }
  }
}


ApplicationInterface::~ApplicationInterface() 
{ }


void ApplicationInterface::
init_communicators(const IntArray& message_lengths,
		   int max_iterator_concurrency)
{
  // Initialize comms for evaluations (partitions of iteratorComm).
  int min_procs_per_eval = procsPerAnalysis; // could add *numAnalysisDrivers
  std::string default_config = "push_up"; // init_eval_comms&init_analysis_comms
  const ParallelLevel& ie_pl = parallelLib.init_evaluation_communicators(
    numEvalServers, min_procs_per_eval, max_iterator_concurrency,
    asynchLocalEvalConcurrency, default_config, evalScheduling);

  set_evaluation_communicators(message_lengths);

  // Initialize communicators for analyses (partitions of evalComm).  This call
  // is protected from an iterator dedicated master in the same way a strategy
  // master never calls init_eval_comms (prevents some warnings in
  // ParallelLibrary::resolve_inputs).
  if ( !ieDedMasterFlag || iteratorCommRank ) {

    const ParallelLevel& ea_pl = parallelLib.init_analysis_communicators(
      numAnalysisServers, procsPerAnalysis, numAnalysisDrivers,
      asynchLocalAnalysisConcurrency, default_config, analysisScheduling);

    set_analysis_communicators();
  }

  // print parallel configuration (prior to configuration checking
  // so that error messages can be more readily debugged)
  if (worldSize > 1)
    parallelLib.print_configuration();

  // check for configuration errors
  init_communicators_checks(max_iterator_concurrency);
}


void ApplicationInterface::
set_communicators(const IntArray& message_lengths, int max_iterator_concurrency)
{
  set_evaluation_communicators(message_lengths);

  // Initialize communicators for analyses (partitions of evalComm).  This call
  // is protected from an iterator dedicated master in the same way a strategy
  // master never calls init_eval_comms (prevents some warnings in
  // ParallelLibrary::resolve_inputs).
  if ( !ieDedMasterFlag || iteratorCommRank )
    set_analysis_communicators();

  // check for configuration errors
  set_communicators_checks(max_iterator_concurrency);
}


void ApplicationInterface::
set_evaluation_communicators(const IntArray& message_lengths)
{
  // Buffer sizes for function evaluation message transfers are estimated in 
  // Model::init_communicators() so that hard-coded MPIUnpackBuffer
  // lengths can be avoided.  This estimation is reperformed on every call to
  // Strategy::run_iterator().  A Bcast is not currently needed since
  // every processor performs the estimation.
  //MPI_Bcast(message_lengths.data(), 4, MPI_INT, 0, iteratorComm);
  lenVarsMessage       = message_lengths[0];
  lenVarsActSetMessage = message_lengths[1];
  lenResponseMessage   = message_lengths[2];
  lenPRPairMessage     = message_lengths[3];

  const ParallelConfiguration& pc = parallelLib.parallel_configuration();
  const ParallelLevel& si_pl = pc.si_parallel_level();
  const ParallelLevel& ie_pl = pc.ie_parallel_level();

  // Pull data from (the lowest) concurrent iterator partition.  The active
  // parallel configuration is managed in Strategy::init_communicators().
  // To do: for the lowest level, will need --siPLIters.end().
  // > This code needs to follow init_evaluation_communicators() in case
  //   a siPLIter copy from the previous parallel configuration is used.
  // > Future concurrent iterator partitioning within nested models would also 
  //   dictate that iterator comm attributes get set at init time rather than
  //   construct time.
  // > It would be beneficial to have suppressOutput available at SysCall/Fork
  //   ApplicInterface construct time so that the corresponding AnalysisCode
  //   flags can be set up once, but they will instead be set at eval time.
  iteratorCommSize = si_pl.server_communicator_size();
  iteratorCommRank = si_pl.server_communicator_rank();
  if (iteratorCommRank // any processor other than rank 0 in iteratorComm
  || (outputLevel == SILENT_OUTPUT && evalCommRank == 0
      && !eaDedMasterFlag && numAnalysisServers < 2))
    suppressOutput = true; // suppress output of fn. eval. echoes

  /* Additional output granularity:
  if (ieMessagePass)                // suppress fn eval output in 
    suppressLowLevelOutput = true;  //   SysCall/Fork/Direct
  if (methodOutput == "quiet")      // suppress scheduling & vars/response
    suppressHighLevelOutput = true; //   output in ApplicationInterface & Model
  */

  // These attributes are set by init_evaluation_communicators and are not 
  // available for use in the constructor.
  ieDedMasterFlag = ie_pl.dedicated_master_flag();
  ieMessagePass   = ie_pl.message_pass();
  numEvalServers  = ie_pl.num_servers(); // update to actual
  evalCommRank    = ie_pl.server_communicator_rank();
  evalCommSize    = ie_pl.server_communicator_size();
  evalServerId    = ie_pl.server_id();
  if (ieDedMasterFlag)
//#ifdef COMM_SPLIT_TO_SINGLE
    multiProcEvalFlag = (ie_pl.processors_per_server() > 1 ||
			 ie_pl.processor_remainder());
//#else
    // want multiProcEvalFlag=true on iterator master when slave evalCommSize>1
    //multiProcEvalFlag = ie_pl.communicator_split_flag();
//#endif
  else // split flag insufficient if 1 server (no split in peer case)
    multiProcEvalFlag = (evalCommSize > 1) ? true : false; // could vary
}


void ApplicationInterface::set_analysis_communicators()
{
  const ParallelConfiguration& pc = parallelLib.parallel_configuration();
  const ParallelLevel& ea_pl = pc.ea_parallel_level();

  // Extract attributes for analysis partitions
  eaDedMasterFlag    = ea_pl.dedicated_master_flag();
  eaMessagePass      = ea_pl.message_pass();
  numAnalysisServers = ea_pl.num_servers(); // update to actual
  analysisCommRank   = ea_pl.server_communicator_rank();
  analysisCommSize   = ea_pl.server_communicator_size();
  analysisServerId   = ea_pl.server_id();
  if (eaDedMasterFlag)
//#ifdef COMM_SPLIT_TO_SINGLE
    multiProcEvalFlag = (ea_pl.processors_per_server() > 1 ||
			 ea_pl.processor_remainder());
//#else
    // want multiProcAnalysisFlag=true on eval master when slave analysis size>1
    //multiProcAnalysisFlag = ea_pl.communicator_split_flag();
//#endif
  else // split flag insufficient if 1 server (no split in peer case)
    multiProcAnalysisFlag = (analysisCommSize > 1) ? true : false; // could vary

  // Set flag for asynch local parallelism of analyses.  In the local asynch
  // case (no message passing), a concurrency specification is interpreted as
  // a limit (default is unlimited).  In the message passing case, the user
  // must explicitly specify analysis concurrency to get hybrid parallelism
  // (default is no analysis concurrency).
  if ( numAnalysisDrivers > 1 && interfaceSynchronization == "asynchronous" &&
       ( asynchLocalAnalysisConcurrency > 1 || ( !eaMessagePass &&
        !asynchLocalAnalysisConcurrency ) ) )
    asynchLocalAnalysisFlag = true;
}


/** Override DirectApplicInterface definition if plug-in to allow batch
    processing in Plugin{Serial,Parallel}DirectApplicInterface.C */
void ApplicationInterface::
init_communicators_checks(int max_iterator_concurrency)
{ } // default is no-op


/** Override DirectApplicInterface definition if plug-in to allow batch
    processing in Plugin{Serial,Parallel}DirectApplicInterface.C */
void ApplicationInterface::
set_communicators_checks(int max_iterator_concurrency)
{ } // default is no-op


bool ApplicationInterface::check_multiprocessor_analysis()
{
  bool err_flag = false;
  // multiprocessor analyses are only valid for synchronous direct interfaces.
  // Neither system calls (synch or asynch), forks (synch or asynch), nor POSIX
  // threads (asynch direct) can share a communicator.  Attempting parallel
  // analyses without a shared communicator can result in correct answers if
  // analysisComm's local leader computes the total result, but it does NOT
  // perform the intended multiprocessor analysis and is therefore misleading
  // and should be explicitly prevented.
  if (multiProcAnalysisFlag) { // not valid for system/fork
    // Note: processors_per_analysis only read by DB for direct interfaces,
    //       so currently should not happen.
    Cerr << "Error: Multiprocessor analyses are not valid with "
	 << interfaceType << " interfaces.\n       Your processor allocation "
	 << "may exceed the concurrency in the problem,\n       requiring a "
	 << "reduction in allocation to eliminate the assignment of\n       "
	 << "excess processors to the analysis level." << std::endl;
    err_flag = true;
  }
  return err_flag;
}


bool ApplicationInterface::check_asynchronous(int max_iterator_concurrency)
{
  bool err_flag = false, asynch_local_eval_flag
    = ( max_iterator_concurrency > 1 &&
	interfaceSynchronization == "asynchronous" &&
	( asynchLocalEvalConcurrency > 1 ||           // captures hybrid mode
	  ( !ieMessagePass && !asynchLocalEvalConcurrency ) ) ); // unlimited

  // Check for asynchronous local evaluations or analyses
  if (asynch_local_eval_flag || asynchLocalAnalysisFlag ) {
    Cerr << "Error: asynchronous capability not supported in "
	 << interfaceType << " interfaces." << std::endl;
    err_flag = true;
  }
  return err_flag;
}


bool ApplicationInterface::
check_multiprocessor_asynchronous(int max_iterator_concurrency)
{
  bool err_flag = false, asynch_local_eval_flag
    = ( max_iterator_concurrency > 1 &&
	interfaceSynchronization == "asynchronous" &&
	( asynchLocalEvalConcurrency > 1 ||           // captures hybrid mode
	  ( !ieMessagePass && !asynchLocalEvalConcurrency ) ) ); // unlimited

  // Performing asynch local concurrency requires a single processor.
  // multiProcEvalFlag & asynchLocalAnalysisConcurrency can coexist provided
  // that evalComm is divided into single-processor analysis servers.
  if ( (multiProcEvalFlag     && asynch_local_eval_flag) ||
       (multiProcAnalysisFlag && asynchLocalAnalysisFlag) ) {
    Cerr << "Error: asynchronous local jobs are not supported for "
	 << "multiprocessor\n       communicator partitions.  Your "
	 << "processor allocation may need adjustment." << std::endl;
    err_flag = true;
  }
  return err_flag;
}


void ApplicationInterface::free_communicators()
{
  // This call is protected from an iterator dedicated master in the same way a
  // strategy master never calls init_eval_comms (prevents some warnings in 
  // ParallelLibrary::resolve_inputs).

  // deallocate partitions of evalComm
  if ( !ieDedMasterFlag || iteratorCommRank )
    parallelLib.free_analysis_communicators();

  // deallocate partitions of iteratorComm
  parallelLib.free_evaluation_communicators();
}


/** The function evaluator for application interfaces.  Called from
    derived_compute_response() and derived_asynch_compute_response() in
    derived Model classes.  If asynch_flag is not set, perform a blocking
    evaluation (using derived_map()).  If asynch_flag is set, add the job
    to the beforeSynchCorePRPQueue queue for execution by one of the
    scheduler routines in synch() or synch_nowait().  Duplicate function
    evaluations are detected with duplication_detect(). */
void ApplicationInterface::map(const Variables& vars, const ActiveSet& set,
			       Response& response, const bool asynch_flag)
{
  ++evalIdCntr; // all calls to map for this interface instance
  if (fineGrainEvalCounters) { // detailed evaluation reporting
    const ShortArray& asv = set.request_vector();
    size_t i, num_fns = asv.size();
    for (i=0; i<num_fns; ++i) {
      short asv_val = asv[i];
      if (asv_val & 1) ++fnValCounter[i];
      if (asv_val & 2) ++fnGradCounter[i];
      if (asv_val & 4) ++fnHessCounter[i];
    }
    if (fnLabels.empty())
      fnLabels = response.function_labels();
  }
  if (outputLevel > SILENT_OUTPUT) {
    Cout << "\n------------------------------\nBegin Function Evaluation " 
	 << std::setw(4) << evalIdCntr;
    // This may be more confusing than helpful:
    //if (evalIdRefPt)
    //  Cout << " (local evaluation " << evalIdCntr - evalIdRefPt << ")";
    Cout << "\n------------------------------\n";
  }
  if (outputLevel > QUIET_OUTPUT)
    Cout << "Parameters for function evaluation " << evalIdCntr << ":\n"
	 << vars << '\n';

  response.active_set(set); // responseActiveSet = set for duplicate search

  // Subdivide ActiveSet for algebraic_mappings() and derived_map()
  Response algebraic_resp, core_resp; // empty handles
  ActiveSet core_set;

  if (algebraicMappings) {
    if (evalIdCntr == 1)
      init_algebraic_mappings(vars, response);
    if (coreMappings) { // both mappings
      ActiveSet algebraic_set;
      asv_mapping(set, algebraic_set, core_set);
      algebraic_resp = Response(algebraic_set);
      if (asynch_flag) {
	ParamResponsePair prp(vars, interfaceId, algebraic_resp, evalIdCntr);
	beforeSynchAlgPRPQueue.insert(prp);
      }
      else
	algebraic_mappings(vars, algebraic_set, algebraic_resp);
      // separate core_resp from response
      core_resp = response.copy();
      core_resp.active_set(core_set);
    }
    else { // algebraic mappings only
      // algebraic_set = incoming set and algebraic_resp = incoming response
      if (asynch_flag) {
	ParamResponsePair prp(vars, interfaceId, response, evalIdCntr);
	beforeSynchAlgPRPQueue.insert(prp);
      }
      else
	algebraic_mappings(vars, set, response);
    }
  }
  else if (coreMappings) { // analysis_driver mappings only
    core_set  = set;
    core_resp = response; // shared rep: no need for response_mapping()
  }

  bool duplicate = false;
  if (coreMappings) {
    if (evalCacheFlag && duplication_detect(vars, core_resp, asynch_flag)) {
      // catches duplication both in data_pairs (core evals already computed)
      // and in beforeSynchCorePRPQueue (core evals queued for processing).
      duplicate = true;
      if (outputLevel > SILENT_OUTPUT)
	Cout << "Duplication detected: analysis_drivers not invoked.\n";
    }
    else {

      //if ( partial_duplication_detect(vars, set, core_resp) ) {
        // sets augmentFlag (for use by Response::read), adds to core_resp,
        // and decrements the asv to be used in derived_map, but saves the  
        // original asv for resetting once everything's reintegrated.  The 
        // original asv must be restored prior to any I/O of the core_resp.
      //}

      // For new evaluations, manage the user's active_set_vector specification.
      //    on: asv seen by user's interface may change on each eval (default)
      //   off: asv seen by user's interface is constant for all evals
      if (!asvControlFlag) { // set ASV's to defaultASV for the mapping
	core_set.request_vector(defaultASV);
	core_resp.active_set(core_set);
      }

      if (asynch_flag) { // multiple simultaneous evals. (local or parallel)
	// use this constructor since deep copies of vars/response are needed
	ParamResponsePair prp(vars, interfaceId, core_resp, evalIdCntr);
	beforeSynchCorePRPQueue.insert(prp);
	// jobs are not queued until call to synch() to allow self-scheduling.
	// Response data headers & data_pair list insertion appear in synch().
      }
      else { // local synchronous evaluation

	// bcast the job to other processors within peer 1 (if required)
	if (multiProcEvalFlag)
	  broadcast_evaluation(evalIdCntr, vars, core_set);

	//common_input_filtering(vars);

	currEvalId = evalIdCntr;
	try { derived_map(vars, core_set, core_resp, currEvalId); }

	catch(int fail_code) { // value of fail_code not currently used.
	  //Cout << "Caught int in map" << std::endl;
	  manage_failure(vars, core_set, core_resp, currEvalId);
	}

	//common_output_filtering(core_resp);

	if (evalCacheFlag || restartFileFlag) {
	  // manage shallow/deep copy of vars/response with evalCacheFlag
	  ParamResponsePair prp(vars, interfaceId, core_resp, currEvalId,
				evalCacheFlag);
	  if (evalCacheFlag)   data_pairs.insert(prp);
	  if (restartFileFlag) write_restart << prp;
	}
      }
    }
  }

  if (!duplicate) {
    ++newEvalIdCntr; // nonduplicate evaluations (used ONLY in fn eval summary)
    if (fineGrainEvalCounters) { // detailed evaluation reporting
      const ShortArray& asv = set.request_vector();
      size_t i, num_fns = asv.size();
      for (i=0; i<num_fns; ++i) {
	short asv_val = asv[i];
	if (asv_val & 1) ++newFnValCounter[i];
	if (asv_val & 2) ++newFnGradCounter[i];
	if (asv_val & 4) ++newFnHessCounter[i];
      }
    }
  }

  if (asynch_flag) {
    if (!duplicate && outputLevel > SILENT_OUTPUT)
      Cout << "(Asynchronous job " << evalIdCntr << " added to queue)\n";
  }
  else {
    if (algebraicMappings && coreMappings)
      response_mapping(algebraic_resp, core_resp, response);

    if (outputLevel > QUIET_OUTPUT) {
      if (duplicate)
	Cout << "\nActive response data retrieved from database";
      else
	Cout << "\nActive response data for function evaluation " << evalIdCntr;
      Cout << ":\n" << response << std::endl;
    }
  }
}


/** Called from map() to check incoming evaluation request for
    duplication with content of data_pairs and beforeSynchCorePRPQueue.  
    If duplication is detected, return true, else return false.  Manage
    bookkeeping with historyDuplicateMap and beforeSynchDuplicateMap.
    Note that the list searches can get very expensive if a long list
    is searched on every new function evaluation (either from a large
    number of previous jobs, a large number of pending jobs, or both).
    For this reason, a user request for deactivation of the evaluation
    cache results in a complete bypass of duplication_detect(), even
    though a beforeSynchCorePRPQueue search would still be meaningful.
    Since the intent of this request is to streamline operations, both
    list searches are bypassed. */
bool ApplicationInterface::
duplication_detect(const Variables& vars, Response& response,
		   const bool asynch_flag)
{
  // check data_pairs list
  Response desired_resp;

  // The incoming response's responseActiveSet was updated in map(), but
  // the rest of response is out-of-date (the previous fn. eval).
  if (lookup_by_val(data_pairs, interfaceId, vars, response.active_set(),
		    desired_resp)) {
    // due to id_vars_set_compare, the desired response set could be a
    // subset of the data_pairs response -> use update().
    response.update(desired_resp);
    if (asynch_flag) // asynch case: bookkeep
      historyDuplicateMap[evalIdCntr] = response.copy();
    return true; // Duplication detected.
  }
  // check beforeSynchCorePRPQueue (if asynchronous)
  if (asynch_flag) {
    PRPQueueHIter prp_hash_iter
      = lookup_by_val(beforeSynchCorePRPQueue, interfaceId, vars,
		      response.active_set());
    if ( prp_hash_iter != hashedQueueEnd(beforeSynchCorePRPQueue) ) {
      // Duplication detected: bookkeep
      beforeSynchDuplicateMap[evalIdCntr]
	= std::make_pair(prp_hash_iter, response.copy());
      return true; // Duplication detected.
    }
  }
  return false; // Duplication not detected.
}


/** This function provides blocking synchronization for all cases of
    asynchronous evaluations, including the local asynchronous case
    (background system call, nonblocking fork, & multithreads), the
    message passing case, and the hybrid case.  Called from
    derived_synchronize() in derived Model classes. */
const IntResponseMap& ApplicationInterface::synch()
{
  // Process history duplicates (see duplication_detect) since response data 
  // has been extracted from the data_pairs list.  These duplicates are not 
  // written to data_pairs or write_restart.
  rawResponseMap.clear();
  rawResponseMap = historyDuplicateMap;
  historyDuplicateMap.clear();

  // Process nonduplicate evaluations for either the message passing or local 
  // asynchronous case.
  size_t core_prp_entries = beforeSynchCorePRPQueue.size(), num_synch_jobs
    = (coreMappings) ? core_prp_entries : beforeSynchAlgPRPQueue.size();
  Cout << "\nBlocking synchronize of " << num_synch_jobs
       << " asynchronous evaluations" << std::endl; // disincludes duplicates
  if (core_prp_entries) {
    if (ieMessagePass) {
      if (ieDedMasterFlag)
        self_schedule_evaluations();   // single or multi-processor servers
      else 
        static_schedule_evaluations(); // single or multi-processor servers
    }
    else // local to proc.
      if (asynchLocalEvalStatic && asynchLocalEvalConcurrency > 1)
	asynchronous_local_evaluations_static(beforeSynchCorePRPQueue);
      else
	asynchronous_local_evaluations(beforeSynchCorePRPQueue);
  }

  // Now that beforeSynchCorePRPQueue processing is complete, process duplicates
  // detected within beforeSynchCorePRPQueue (see duplication_detect).
  for (std::map<int, std::pair<PRPQueueHIter, Response> >::const_iterator
       bsd_iter  = beforeSynchDuplicateMap.begin();
       bsd_iter != beforeSynchDuplicateMap.end(); bsd_iter++) {
    // due to id_vars_set_compare, the desired response set could be a subset
    // of the beforeSynchCorePRPQueue duplicate response -> use update().
    rawResponseMap[bsd_iter->first] = (bsd_iter->second).second;
    rawResponseMap[bsd_iter->first].update(
      (bsd_iter->second).first->prp_response());
  }
  beforeSynchDuplicateMap.clear();
  beforeSynchCorePRPQueue.clear();

  // Merge core mappings and algebraic mappings into rawResponseMap
  if (algebraicMappings) { // complete all algebraic jobs and overlay
    for (PRPQueueIter alg_prp_it = beforeSynchAlgPRPQueue.begin();
	 alg_prp_it != beforeSynchAlgPRPQueue.end(); alg_prp_it++) {
      Response alg_response = alg_prp_it->prp_response();
      algebraic_mappings(alg_prp_it->prp_parameters(),
			 alg_prp_it->active_set(), alg_response);
      if (coreMappings) {
	Response& response = rawResponseMap[alg_prp_it->eval_id()];
	response_mapping(alg_response, response, response);
      }
      else
	rawResponseMap[alg_prp_it->eval_id()] = alg_response;
    }
  }
  beforeSynchAlgPRPQueue.clear();

  if (outputLevel > QUIET_OUTPUT) // output completed responses
    for (IntRespMCIter rr_iter = rawResponseMap.begin();
	 rr_iter != rawResponseMap.end(); ++rr_iter)
      Cout << "\nActive response data for function evaluation "
	   << rr_iter->first << ":\n" << rr_iter->second;

  return rawResponseMap;
}


/** This function will eventually provide nonblocking synchronization
    for all cases of asynchronous evaluations, however it currently
    supports only the local asynchronous case since nonblocking
    message passing schedulers have not yet been implemented.  Called
    from derived_synchronize_nowait() in derived Model classes. */
const IntResponseMap& ApplicationInterface::synch_nowait()
{
  rawResponseMap.clear();

  // Test nonduplicate evaluations and add completions to rawResponseMap
  size_t core_prp_entries = beforeSynchCorePRPQueue.size();
  if (headerFlag) { // set whenever an executed job (nonduplicate) completes
    size_t num_synch_jobs = (coreMappings) ?
      core_prp_entries : beforeSynchAlgPRPQueue.size();
    Cout << "\nNonblocking synchronize of " << num_synch_jobs
         << " asynchronous evaluations" << std::endl; // disincludes duplicates
  }
  if (core_prp_entries) {
    if (ieMessagePass) {
      //if (ieDedMasterFlag)
      //  self_schedule_evaluations_nowait();
      //else 
      //  static_schedule_evaluations_nowait();
      Cerr << "Error: synch_nowait not yet available in message passing mode."
           << std::endl;
      abort_handler(-1);
    }
    else 
      asynchronous_local_evaluations_nowait(beforeSynchCorePRPQueue);
  }
  headerFlag = !rawResponseMap.empty();

  // Since beforeSynchCorePRPQueue processing will not in general be completed, 
  // process duplicates listed in beforeSynchDuplicateMap only if the 
  // original/nonduplicate beforeSynchCorePRPQueue job is complete.
  for (std::map<int, std::pair<PRPQueueHIter, Response> >::iterator 
       bsd_iter  = beforeSynchDuplicateMap.begin();
       bsd_iter != beforeSynchDuplicateMap.end(); bsd_iter++) {
    const ParamResponsePair& scheduled_pr = *(bsd_iter->second).first;
    if (rawResponseMap.find(scheduled_pr.eval_id()) != rawResponseMap.end()) {
      // due to id_vars_set_compare, the desired response set could be
      // a subset of the duplicate response -> use update().
      Response& response = (bsd_iter->second).second;
      response.update(scheduled_pr.prp_response());
      rawResponseMap[bsd_iter->first] = response;
    }
  }

  // Process history duplicates (see duplication_detect).  In the _nowait case,
  // this goes after the schedulers so as to not interfere with rawResponseMap
  // usage in the schedulers and after the beforeSynchDuplicates in order to 
  // streamline their rawResponseMap searches.  Note: since data_pairs is
  // checked first in duplication_detect(), it is not possible to have a
  // beforeSynchDuplicateMap entry that references a historyDuplicateMap entry.
  rawResponseMap.insert(historyDuplicateMap.begin(), historyDuplicateMap.end());
  historyDuplicateMap.clear();

  // Merge core mappings and algebraic mappings into rawResponseMap
  if (coreMappings && algebraicMappings) { // update completed core jobs
    for (IntRespMIter rr_iter = rawResponseMap.begin();
	 rr_iter != rawResponseMap.end(); ++rr_iter) {
      PRPQueueIter alg_prp_it
	= lookup_by_eval_id(beforeSynchAlgPRPQueue, rr_iter->first);
      Response alg_response = alg_prp_it->prp_response();         // shared rep
      algebraic_mappings(alg_prp_it->prp_parameters(),
			 alg_prp_it->active_set(), alg_response); // update rep
      response_mapping(alg_response, rr_iter->second, rr_iter->second);
      beforeSynchAlgPRPQueue.erase(alg_prp_it);
    }
  }
  else if (algebraicMappings) { // complete all algebraic jobs
    for (PRPQueueIter alg_prp_it = beforeSynchAlgPRPQueue.begin();
	 alg_prp_it != beforeSynchAlgPRPQueue.end(); alg_prp_it++) {
      Response raw_response = rawResponseMap[alg_prp_it->eval_id()]
	= alg_prp_it->prp_response();                             // shared rep
      algebraic_mappings(alg_prp_it->prp_parameters(),
			 alg_prp_it->active_set(), raw_response); // update rep
    }
    beforeSynchAlgPRPQueue.clear();
  }

  for (IntRespMCIter rr_iter = rawResponseMap.begin();
       rr_iter != rawResponseMap.end(); ++rr_iter) {
    int fn_eval_id = rr_iter->first;
    // output completed responses
    if (outputLevel > QUIET_OUTPUT)
      Cout << "\nActive response data for function evaluation " << fn_eval_id
	   << ":\n" << rr_iter->second;
    // clean up bookkeeping
    if (coreMappings) {
      PRPQueueIter prp_iter
	= lookup_by_eval_id(beforeSynchCorePRPQueue, fn_eval_id);
      if (prp_iter != beforeSynchCorePRPQueue.end()) // duplicates not in list
	beforeSynchCorePRPQueue.erase(prp_iter);
      beforeSynchDuplicateMap.erase(fn_eval_id); // if present
    }
  }

  return rawResponseMap;
}


/** This code is called from synch() to provide the master portion of
    a master-slave algorithm for the dynamic self-scheduling of
    evaluations among slave servers.  It performs no evaluations
    locally and matches either serve_evaluations_synch() or
    serve_evaluations_asynch() on the slave servers, depending on the
    value of asynchLocalEvalConcurrency.  Self-scheduling approach
    assigns jobs in 2 passes.  The 1st pass gives each server the same
    number of jobs (equal to asynchLocalEvalConcurrency).  The 2nd
    pass assigns the remaining jobs to slave servers as previous jobs
    are completed.  Single- and multilevel parallel use intra- and
    inter-communicators, respectively, for send/receive.  Specific
    syntax is encapsulated within ParallelLibrary. */
void ApplicationInterface::self_schedule_evaluations()
{
  int num_jobs  = beforeSynchCorePRPQueue.size();
  int capacity  = (asynchLocalEvalConcurrency) ?
                   asynchLocalEvalConcurrency * numEvalServers : numEvalServers;
  int num_sends = (capacity < num_jobs) ? capacity : num_jobs;
  Cout << "First pass: assigning " << num_sends << " jobs among " 
       << numEvalServers << " servers\n";

  // only need num_sends entries (not num_jobs) due to reuse
  MPIPackBuffer*   send_buffers  = new MPIPackBuffer   [num_sends];
  MPIUnpackBuffer* recv_buffers  = new MPIUnpackBuffer [num_sends];
  MPI_Request      send_request; // only 1 needed since no test/wait on sends
  MPI_Request*     recv_requests = new MPI_Request     [num_sends];

  // send data & post receives for 1st set of jobs
  int i, server_id, fn_eval_id;
  PRPQueueIter prp_iter;
  for (i=0, prp_iter = beforeSynchCorePRPQueue.begin(); i<num_sends;
       ++i, ++prp_iter) {
    server_id  = i%numEvalServers + 1; // from 1 to numEvalServers
    fn_eval_id = prp_iter->eval_id();
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Master assigning function evaluation " << fn_eval_id 
           << " to server " << server_id << '\n';
    send_buffers[i] << prp_iter->prp_parameters() << prp_iter->active_set();
    // pre-post nonblocking receives (to prevent any message buffering)
    recv_buffers[i].resize(lenResponseMessage);
    parallelLib.irecv_ie(recv_buffers[i], server_id, fn_eval_id,
                         recv_requests[i]);
    // nonblocking sends: master quickly assigns first num_sends jobs.  
    parallelLib.isend_ie(send_buffers[i], server_id, fn_eval_id, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }

  // self-schedule remaining jobs
  if (num_sends < num_jobs) {
    Cout << "Second pass: self-scheduling " << num_jobs-num_sends 
         << " remaining jobs\n";
    int send_cntr = num_sends, recv_cntr = 0, out_count;
    MPI_Status* status_array = new MPI_Status [num_sends];
    int* index_array = new int [num_sends];
    PRPQueueIter q_it;
    while (recv_cntr < num_jobs) {
      if (outputLevel > SILENT_OUTPUT)
        Cout << "Waiting on completed jobs" << std::endl;
      parallelLib.waitsome(num_sends, recv_requests, out_count, index_array, 
			   status_array);
      recv_cntr += out_count;
      for (i=0; i<out_count; ++i) {
        int index  = index_array[i]; // index of recv_request that completed
        server_id  = index%numEvalServers + 1; // from 1 to numEvalServers
        fn_eval_id = status_array[i].MPI_TAG;
        if (outputLevel > SILENT_OUTPUT)
          Cout << "job " << fn_eval_id << " has returned from slave server "
               << server_id << '\n';

        // Process buffer and insert into restart file ASAP.  Avoid
        // multiple key-value lookups.
        q_it = lookup_by_eval_id(beforeSynchCorePRPQueue, fn_eval_id);
	Response raw_response
	  = rawResponseMap[fn_eval_id] = q_it->prp_response(); // shared rep
        recv_buffers[index] >> raw_response;                   // rep updated
        if (evalCacheFlag)   data_pairs.insert(*q_it);
        if (restartFileFlag) write_restart << *q_it;

        if (send_cntr < num_jobs) {
          fn_eval_id = prp_iter->eval_id();
          send_buffers[index].reset(); // reuse send_buffers & recv_requests
          send_buffers[index] << prp_iter->prp_parameters()
			      << prp_iter->active_set();
          if (outputLevel > SILENT_OUTPUT)
            Cout << "Master assigning function evaluation " << fn_eval_id 
                 << " to server " << server_id << '\n';
	  // pre-post receives
          recv_buffers[index].reset();
          parallelLib.irecv_ie(recv_buffers[index], server_id, fn_eval_id, 
                               recv_requests[index]);
          parallelLib.isend_ie(send_buffers[index], server_id, fn_eval_id,
                               send_request);
          parallelLib.free(send_request); // no test/wait on send_request
          ++send_cntr;
          ++prp_iter;
        }
      }
    }
    delete [] status_array;
    delete [] index_array;
  }
  else { // all jobs assigned in first pass
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Waiting on all jobs" << std::endl;
    parallelLib.waitall(num_jobs, recv_requests);
    // All buffers received, now generate rawResponseMap
    for (i=0, prp_iter = beforeSynchCorePRPQueue.begin(); i<num_jobs;
         ++i, ++prp_iter) {
      // Since Response uses the handle-body idiom, the first assignment
      // results in a shared representation (body), which is updated (for both
      // handles) from the recv_buffer.
      Response raw_response = rawResponseMap[prp_iter->eval_id()]
	= prp_iter->prp_response();    // shared rep
      recv_buffers[i] >> raw_response; // rep updated

      if (evalCacheFlag)   data_pairs.insert(*prp_iter);
      if (restartFileFlag) write_restart << *prp_iter;
    }
  }
  // deallocate MPI & buffer arrays
  delete [] send_buffers;
  delete [] recv_buffers;
  delete [] recv_requests;
}


/** This code runs on the iteratorCommRank 0 processor (the iterator)
    and is called from synch() in order to assign a static schedule.
    It matches serve_evaluations_peer() for any other processors
    within the first evaluation partition and
    serve_evaluations_{synch,asynch}() for all other evaluation
    partitions (depending on asynchLocalEvalConcurrency).  It performs
    function evaluations locally for its portion of the static
    schedule using either asynchronous_local_evaluations() or
    synchronous_local_evaluations().  Single-level and multilevel
    parallel use intra- and inter-communicators, respectively, for
    send/receive.  Specific syntax is encapsulated within
    ParallelLibrary.  The iteratorCommRank 0 processor assigns the
    static schedule since it is the only processor with access to
    beforeSynchCorePRPQueue (it runs the iterator and calls
    synchronize).  The alternate design of each peer selecting its own
    jobs using the modulus operator would be applicable if execution
    of this function (and therefore the job list) were distributed. */
void ApplicationInterface::static_schedule_evaluations()
{
  // rounding down num_peer1_jobs offloads this processor (which has additional
  // work relative to other peers), but results in a few more passed messages.
  int num_jobs       = beforeSynchCorePRPQueue.size(), 
      num_peer1_jobs = (int)std::floor((Real)num_jobs/numEvalServers),
      num_sends      = num_jobs - num_peer1_jobs;
  Cout << "Static schedule: assigning " << num_jobs << " jobs among " 
       << numEvalServers << " peers\n";
  MPIPackBuffer*   send_buffers  = new MPIPackBuffer   [num_sends];
  MPIUnpackBuffer* recv_buffers  = new MPIUnpackBuffer [num_sends];
  MPI_Request      send_request; // only 1 needed since no test/wait on sends
  MPI_Request*     recv_requests = new MPI_Request     [num_sends];
  int i, server_id, fn_eval_id;

  // Peer 1 retains the first num_peer1_jobs and spreads the rest to
  // peers 2 through n in a round-robin assignment.  Since this is a
  // static schedule, all nonretained jobs are assigned now.  This
  // assignment is not dependent on the capacity of the other peers
  // (i.e., on whether they run serve_evaluation_synch or
  // serve_evaluation_asynch).

  PRPQueueIter prp_iter = beforeSynchCorePRPQueue.begin();

  std::advance(prp_iter, num_peer1_jobs); // offset PRP list by num_peer1_jobs
  PRPQueueIter prp_iter_save = prp_iter;
  for (i=0; i<num_sends; ++i, ++prp_iter) {
    server_id = i%(numEvalServers-1) + 1; // 1 to numEvalServers-1
    fn_eval_id = prp_iter->eval_id();
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Peer 1 assigning function evaluation " << fn_eval_id 
           << " to peer " << server_id+1 << '\n';
    send_buffers[i] << prp_iter->prp_parameters() << prp_iter->active_set();
    // pre-post nonblocking receives
    recv_buffers[i].resize(lenResponseMessage);
    parallelLib.irecv_ie(recv_buffers[i], server_id, fn_eval_id,
                         recv_requests[i]);
    // post nonblocking sends
    parallelLib.isend_ie(send_buffers[i], server_id, fn_eval_id, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }

  // Perform computation for first num_peer1_jobs jobs on peer 1.  Default 
  // behavior is synchronous evaluation of jobs on each peer since the message-
  // passing parallelism is dominant.  Only if asynchLocalEvalConcurrency > 1
  // do we get the hybrid parallelism of asynch jobs on each peer.
  PRPQueue local_prp_queue(beforeSynchCorePRPQueue.begin(), prp_iter_save);
  if (asynchLocalEvalConcurrency > 1) { // multiple asynch jobs on each peer
    Cout << "Peer 1 self-scheduling " << num_peer1_jobs << " local jobs\n";
    if (asynchLocalEvalStatic && outputLevel > SILENT_OUTPUT)
      Cout << "Warning: static scheduling within asynchronous local evaluation "
	   << "not supported\n in hybrid MPI/local parallelism mode. Using "
	   << "local self-scheduling." << std::endl;
    asynchronous_local_evaluations(local_prp_queue);
  }
  else { // 1 synchronous job at a time
    Cout << "Peer 1 evaluating " << num_peer1_jobs << " local jobs\n";
    synchronous_local_evaluations(local_prp_queue);
  }
  // reassign used to be required for beforeSynchDuplicates to work properly in
  // synch(), but is now unnecessary due to handle-body representation sharing
  // between local_prp_queue and beforeSynchCorePRPQueue.
  //for (i=0; i<num_peer1_jobs; ++i)
  //  beforeSynchCorePRPQueue[i].prp_response(
  //    local_prp_queue[i].prp_response());

  if (num_sends) { // Retrieve results from peers
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Waiting on assigned jobs" << std::endl;
    parallelLib.waitall(num_sends, recv_requests);

    // All buffers received, now generate rawResponseMap
    prp_iter = beforeSynchCorePRPQueue.begin();
    std::advance(prp_iter, num_peer1_jobs); // offset PRP list by num_peer1_jobs
    for (i=0; i<num_sends; ++i, ++prp_iter) {
      // operator= shares representation (body) which is updated for both
      // handles by operator>>
      Response raw_response = rawResponseMap[prp_iter->eval_id()]
	= prp_iter->prp_response();
      recv_buffers[i] >> raw_response;

      if (evalCacheFlag)   data_pairs.insert(*prp_iter);
      if (restartFileFlag) write_restart << *prp_iter;
    }
  }

  // deallocate MPI & buffer arrays
  delete [] send_buffers;
  delete [] recv_buffers;
  delete [] recv_requests;
}


/** This function provides blocking synchronization for the local
    asynch case (background system call, nonblocking fork, or
    threads).  It can be called from synch() for a complete local
    scheduling of all asynchronous jobs or from
    static_schedule_evaluations() to perform a local portion of the
    total job set.  It uses the derived_map_asynch() to initiate
    asynchronous evaluations and derived_synch() to capture completed
    jobs, and mirrors the self_schedule_evaluations() message passing
    scheduler as much as possible (derived_synch() is modeled after
    MPI_Waitsome()). */
void ApplicationInterface::
asynchronous_local_evaluations(PRPQueue& local_prp_queue)
{
  PRPQueue active_prp_queue; PRPQueueIter prp_iter;
  size_t i, num_sends; int fn_eval_id, num_jobs = local_prp_queue.size();
  if (multiProcEvalFlag)
    parallelLib.bcast_e(num_jobs);
  if (asynchLocalEvalConcurrency) // concurrency limited by user spec.
    num_sends = (asynchLocalEvalConcurrency < num_jobs) ? 
      asynchLocalEvalConcurrency : num_jobs;
  else // default: no limit, launch all jobs in first pass
    num_sends = num_jobs;
  Cout << "First pass: initiating " << num_sends << " asynchronous jobs\n";
  for (i=0, prp_iter = local_prp_queue.begin(); i<num_sends; ++i, ++prp_iter) {
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Initiating function evaluation " << prp_iter->eval_id() << '\n';
    // bcast job to other processors within peer 1 (added for direct plugins)
    if (multiProcEvalFlag)
      broadcast_evaluation(*prp_iter);
    active_prp_queue.insert(*prp_iter);
    derived_map_asynch(*prp_iter);
  }

  if (num_sends < num_jobs)
    Cout << "Second pass: self-scheduling " << num_jobs-num_sends 
         << " remaining jobs\n";

  size_t send_cntr = num_sends, recv_cntr = 0;
  while (recv_cntr < num_jobs) {
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Waiting on completed jobs" << std::endl;
    completionSet.clear();
    derived_synch(active_prp_queue); // rebuild completionSet from active queue
    recv_cntr += completionSet.size();
    PRPQueueIter q_it;
    for (ISCIter id_iter = completionSet.begin();
	 id_iter != completionSet.end(); ++id_iter) {
      fn_eval_id = *id_iter;
      if (outputLevel > SILENT_OUTPUT)
        Cout << "Function evaluation " << fn_eval_id << " has completed\n";

      // Insert into restart file ASAP. Do not wait for loop below.
      q_it = lookup_by_eval_id(active_prp_queue, fn_eval_id);
      if (q_it == active_prp_queue.end()) {
	Cerr << "Error: failure in eval id lookup in ApplicationInterface::"
	     << "asynchronous_local_evaluations()." << std::endl;
	abort_handler(-1);
      }
      else {
	if (evalCacheFlag)   data_pairs.insert(*q_it);
	if (restartFileFlag) write_restart << *q_it;
	active_prp_queue.erase(q_it);
      }

      if (send_cntr < num_jobs) {
	int fn_eval_id = prp_iter->eval_id();
        if (outputLevel > SILENT_OUTPUT)
          Cout << "Initiating function evaluation " << fn_eval_id << '\n';
	// bcast job to other procs within peer 1 (added for direct plugins)
	if (multiProcEvalFlag)
	  broadcast_evaluation(*prp_iter);
	active_prp_queue.insert(*prp_iter);
        derived_map_asynch(*prp_iter);
        ++send_cntr;
        ++prp_iter;
      }
    }
  }

  // All jobs complete, now generate rawResponseMap
  for (prp_iter  = local_prp_queue.begin();
       prp_iter != local_prp_queue.end(); ++prp_iter)
    rawResponseMap[prp_iter->eval_id()] = prp_iter->prp_response();
  //clear_bookkeeping(); // clear any bookkeeping lists in derived classes
}


/** Statically-scheduled counterpart to asynchronous_local_evaluations.
    This scheduling policy specifically ensures that a completed
    asynchronous evaluation eval_id is replaced with an equivalent one,
    modulo asynchLocalEvalConcurrency.  Designed to help with parallel
    tiling.  A disadvantage of this scheduling policy is that is could
    leave local asynchronous worker "servers" idle in parsing the
    local_prp_queue, e.g., when restarting and some evals are already 
    complete.  In fact, anytime this function is called with non-contiguous
    eval_id's, the full possible concurrency won't be leveraged.

    This is currently only supported when DAKOTA is running in serial.
    Supporting in the MPI static / asynch local hybrid mode would require
    MPI static schedule that is either fully round-robin or fully block
    scheduled, not the present hybrid.  It is not clear how to support
    this in the MPI self scheduled / asynch local hybrid mode.

    If local evaluation concurrency is unlimited, this fn is not needed. */
void ApplicationInterface::
asynchronous_local_evaluations_static(PRPQueue& local_prp_queue)
{
  // TODO: could remove tests for asynchLocalEvalConcurrency

  // special data for static-scheduling case
  int server_num, first_eval_id = local_prp_queue.begin()->eval_id();
  // need to track all completed evaluations, unlike nowait
  IntSet allCompleted;
  // in the wait case, always reset the job map to zero
  if (localServerJobMap.size() != asynchLocalEvalConcurrency)
    localServerJobMap.resize(asynchLocalEvalConcurrency);
  localServerJobMap.assign(asynchLocalEvalConcurrency, 0);

  PRPQueue active_prp_queue; PRPQueueIter prp_iter;
  size_t i, num_sends;
  int fn_eval_id, num_jobs = local_prp_queue.size(), num_active = 0;
  if (multiProcEvalFlag)
    parallelLib.bcast_e(num_jobs);
  if (asynchLocalEvalConcurrency) // concurrency limited by user spec.
    num_sends = (asynchLocalEvalConcurrency < num_jobs) ? 
      asynchLocalEvalConcurrency : num_jobs;
  else // default: no limit, launch all jobs in first pass
    num_sends = num_jobs;
  Cout << "First pass: initiating at most " << num_sends 
       << " asynchronous jobs\n";

  // have to handle the case of non-contiguous eval IDs as it could
  // happen with restart; fill jobs until all servers busy or at end of queue
  // this is similar to nowait case
  for (prp_iter  = local_prp_queue.begin();
       prp_iter != local_prp_queue.end(); ++prp_iter) {

    fn_eval_id = prp_iter->eval_id();
    server_num = (fn_eval_id - 1) % asynchLocalEvalConcurrency;
    // only schedule if local "server" not busy
    if (localServerJobMap[server_num] == 0) {
      if (outputLevel > SILENT_OUTPUT)
	Cout << "Initiating function evaluation " << fn_eval_id << '\n';
      // bcast job to other processors within peer 1 (added for direct plugins)
      if (multiProcEvalFlag)
	broadcast_evaluation(*prp_iter);
      active_prp_queue.insert(*prp_iter); ++num_active;
      derived_map_asynch(*prp_iter);
      localServerJobMap[server_num] = fn_eval_id;
    }
    if (asynchLocalEvalConcurrency && num_active == asynchLocalEvalConcurrency)
      break;
  }

  if (num_active < num_jobs)
    Cout << "Second pass: static-scheduling " << num_jobs - num_active
	 << " remaining jobs\n";

  size_t recv_cntr = 0;
  while (recv_cntr < num_jobs) {
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Waiting on completed jobs" << std::endl;
    completionSet.clear();
    derived_synch(active_prp_queue); // rebuilds completionSet
    recv_cntr += completionSet.size();
    PRPQueueIter q_it;
    for (ISCIter id_iter = completionSet.begin();
	 id_iter != completionSet.end(); ++id_iter) {
      fn_eval_id = *id_iter;
      if (outputLevel > SILENT_OUTPUT)
        Cout << "Function evaluation " << fn_eval_id << " has completed\n";

      // Insert into restart file ASAP. Do not wait for loop below.
      q_it = lookup_by_eval_id(active_prp_queue, fn_eval_id);
      if (q_it == active_prp_queue.end()) {
	Cerr << "Error: failure in eval id lookup in ApplicationInterface::"
	     << "asynchronous_local_evaluations_static()." << std::endl;
	abort_handler(-1);
      }
      else {
	if (evalCacheFlag)   data_pairs.insert(*q_it);
	if (restartFileFlag) write_restart << *q_it;
	active_prp_queue.erase(q_it); --num_active;
      }
      // record the completion and free the local "server"
      allCompleted.insert(fn_eval_id);
      server_num = (fn_eval_id - 1) % asynchLocalEvalConcurrency;
      localServerJobMap[server_num] = 0;
    }
     
    // see if there's another job for this server
    // should probably split out to prevent running the loop a ton
    for (prp_iter = local_prp_queue.begin(); prp_iter != local_prp_queue.end(); 
	 ++prp_iter) {

      fn_eval_id = prp_iter->eval_id();
      
      // see if this queue request still needs scheduling
      if (lookup_by_eval_id(active_prp_queue, fn_eval_id) ==
	  active_prp_queue.end() && !allCompleted.count(fn_eval_id) ) {

	server_num = (fn_eval_id - 1) % asynchLocalEvalConcurrency;
	// only schedule if local "server" not busy
	if (localServerJobMap[server_num] == 0) {
	  if (outputLevel > SILENT_OUTPUT)
	    Cout << "Initiating function evaluation " << fn_eval_id << '\n';
	  // bcast job to other procs within peer 1 (added for direct plugins)
	  if (multiProcEvalFlag)
	    broadcast_evaluation(*prp_iter);
	  active_prp_queue.insert(*prp_iter); ++num_active;
	  derived_map_asynch(*prp_iter);
	  localServerJobMap[server_num] = fn_eval_id;
	}
	if (asynchLocalEvalConcurrency &&
	    num_active == asynchLocalEvalConcurrency)
	  break;
      }
    }
    
  } // end while

  // All jobs complete, now generate rawResponseMap
  for (prp_iter  = local_prp_queue.begin();
       prp_iter != local_prp_queue.end(); ++prp_iter)
    rawResponseMap[prp_iter->eval_id()] = prp_iter->prp_response();
  //clear_bookkeeping(); // clear any bookkeeping lists in derived classes
}


/** This function provides nonblocking synchronization for the local
    asynch case (background system call, nonblocking fork, or
    threads).  It is called from synch_nowait() and passed the
    complete set of all asynchronous jobs (beforeSynchCorePRPQueue).
    It uses derived_map_asynch() to initiate asynchronous evaluations
    and derived_synch_nowait() to capture completed jobs in
    nonblocking mode.  It mirrors a nonblocking message passing
    scheduler as much as possible (derived_synch_nowait() modeled
    after MPI_Testsome()).  The result of this function is
    rawResponseMap, which uses eval_id as a key.  It is assumed that
    the incoming local_prp_queue contains only active and new jobs - 
    i.e., all completed jobs are cleared by synch_nowait(). 

    Also supports asynchronous local evaluations with static
    scheduling.  This scheduling policy specifically ensures that a
    completed asynchronous evaluation eval_id is replaced with an
    equivalent one, modulo asynchLocalEvalConcurrency.  In the nowait
    case, this could render some servers idle if evaluations don't
    come in eval_id order or some evaluations are cancelled by the
    caller in between calls. If this function is called with unlimited
    local eval concurrency, the static scheduling request is ignored. */
void ApplicationInterface::
asynchronous_local_evaluations_nowait(PRPQueue& local_prp_queue)
{
  
  // special data for static scheduling case; only reset job map on first call
  int server_num;
  if (asynchLocalEvalStatic && 
      localServerJobMap.size() != asynchLocalEvalConcurrency) {
    localServerJobMap.resize(asynchLocalEvalConcurrency);
    localServerJobMap.assign(asynchLocalEvalConcurrency, 0);
  }

  // Step 1: launch any new jobs up to asynchLocalEvalConcurrency limit (if 
  // specified.
  int fn_eval_id, num_jobs = local_prp_queue.size();
  if (multiProcEvalFlag)
    parallelLib.bcast_e(num_jobs);
  PRPQueue active_prp_queue; PRPQueueIter prp_iter; ISCIter id_iter;
  // reconstruct active_prp_queue if jobs are running from previous _nowait()
  if (!runningSet.empty()) {
    for (id_iter = runningSet.begin(); id_iter != runningSet.end(); ++id_iter) {
      prp_iter = lookup_by_eval_id(local_prp_queue, *id_iter);
      active_prp_queue.insert(*prp_iter);
    }
  }
  for (prp_iter  = local_prp_queue.begin();
       prp_iter != local_prp_queue.end(); ++prp_iter) {
    fn_eval_id = prp_iter->eval_id();
    if ( !runningSet.count(fn_eval_id) && (!asynchLocalEvalConcurrency
       || runningSet.size() < asynchLocalEvalConcurrency) ) {

      if (asynchLocalEvalConcurrency && asynchLocalEvalStatic) {
	// only schedule if local "server" not busy
	server_num = (fn_eval_id - 1) % asynchLocalEvalConcurrency;
	if (localServerJobMap[server_num] == 0) {
	  if (outputLevel > SILENT_OUTPUT)
	    Cout << "Initiating function evaluation " << fn_eval_id << '\n';
	  // bcast job to other procs within peer 1 (added for direct plugins)
	  if (multiProcEvalFlag)
	    broadcast_evaluation(*prp_iter);
	  active_prp_queue.insert(*prp_iter);
	  derived_map_asynch(*prp_iter);
	  runningSet.insert(fn_eval_id);
	  localServerJobMap[server_num] = fn_eval_id;
	}

      }
      else {
	if (outputLevel > SILENT_OUTPUT)
	  Cout << "Initiating function evaluation " << fn_eval_id << '\n';
	if (multiProcEvalFlag)
	  broadcast_evaluation(*prp_iter);
	active_prp_queue.insert(*prp_iter);
	derived_map_asynch(*prp_iter);
	runningSet.insert(fn_eval_id);
      }
    }
  }

  // Step 2: process any completed jobs with derived_synch_nowait
  if (outputLevel == DEBUG_OUTPUT) // explicit debug user specification
    Cout << "Testing for completed jobs\n";
  completionSet.clear();
  derived_synch_nowait(active_prp_queue); // rebuilds completionSet
  size_t completed = completionSet.size();
  for (id_iter  = completionSet.begin();
       id_iter != completionSet.end(); ++id_iter) {
    fn_eval_id = *id_iter;
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Function evaluation " << fn_eval_id << " has completed\n";
    // Insert into restart file ASAP.
    prp_iter = lookup_by_eval_id(active_prp_queue, fn_eval_id);
    if (prp_iter == active_prp_queue.end()) {
      Cerr << "Error: failure in eval id lookup in ApplicationInterface::"
	   << "asynchronous_local_evaluations_nowait()." << std::endl;
      abort_handler(-1);
    }
    else {
      if (evalCacheFlag)   data_pairs.insert(*prp_iter);
      if (restartFileFlag) write_restart << *prp_iter;
      active_prp_queue.erase(prp_iter);
    }
    // move job from active to completed status and update bookkeeping
    rawResponseMap[fn_eval_id] = prp_iter->prp_response();
    runningSet.erase(fn_eval_id);

    // free this local server
    if (asynchLocalEvalConcurrency && asynchLocalEvalStatic) {
      server_num = (fn_eval_id - 1) % asynchLocalEvalConcurrency;
      localServerJobMap[server_num] = 0;
    }
  }

  // Step 3: backfill completed jobs with the next pending jobs (if present)
  if (completed && completed < num_jobs) {
    for (prp_iter  = local_prp_queue.begin();
	 prp_iter != local_prp_queue.end(); ++prp_iter){
      fn_eval_id = prp_iter->eval_id();
      if ( !runningSet.count(fn_eval_id) && !completionSet.count(fn_eval_id) ) {
	if (asynchLocalEvalConcurrency && asynchLocalEvalStatic) {
	  // only schedule if local "server" not busy
	  server_num = (fn_eval_id - 1) % asynchLocalEvalConcurrency;
	  if (localServerJobMap[server_num] == 0) {
	    if (outputLevel > SILENT_OUTPUT)
	      Cout << "Initiating function evaluation " << fn_eval_id << '\n';
	    if (multiProcEvalFlag)
	      broadcast_evaluation(*prp_iter);
	    active_prp_queue.insert(*prp_iter);
	    derived_map_asynch(*prp_iter);
	    runningSet.insert(fn_eval_id);
	    completed--;
	    localServerJobMap[server_num] = fn_eval_id;
	  }
	}
	else {
	  if (outputLevel > SILENT_OUTPUT)
	    Cout << "Initiating function evaluation " << fn_eval_id << '\n';
	  if (multiProcEvalFlag)
	    broadcast_evaluation(*prp_iter);
	  active_prp_queue.insert(*prp_iter);
	  derived_map_asynch(*prp_iter);
	  runningSet.insert(fn_eval_id);
	  completed--; // use as counter & decrement back towards zero
	}
      }
      if (completed == 0)
        break; // only launch enough jobs to backfill
    }
  }
}


/** This function provides blocking synchronization for the local
    synchronous case (foreground system call, blocking fork, or
    procedure call from derived_map()).  It is called from
    static_schedule_evaluations() to perform a local portion of the
    total job set. */
void ApplicationInterface::
synchronous_local_evaluations(PRPQueue& local_prp_queue)
{

  for (PRPQueueIter prp_iter = local_prp_queue.begin();
       prp_iter != local_prp_queue.end(); ++prp_iter) {
    currEvalId              = prp_iter->eval_id();
    const Variables& vars   = prp_iter->prp_parameters();
    const ActiveSet& set    = prp_iter->active_set();
    Response local_response = prp_iter->prp_response(); // shared rep
    if (outputLevel > SILENT_OUTPUT)
      Cout << "Evaluating function evaluation " << currEvalId << std::endl;

    // bcast the job to other processors within peer 1 (if required)
    if (multiProcEvalFlag)
      broadcast_evaluation(*prp_iter);

    try { derived_map(vars, set, local_response, currEvalId); } // synch. local

    catch(int fail_code) {
      manage_failure(vars, set, local_response, currEvalId);
    }

    rawResponseMap[currEvalId] = local_response; // shares local_prp_queue rep
    //prp_iter->prp_response(local_response); // not needed since shared rep
    if (evalCacheFlag)   data_pairs.insert(*prp_iter);
    if (restartFileFlag) write_restart << *prp_iter;
  }
}


void ApplicationInterface::
broadcast_evaluation(int fn_eval_id, const Variables& vars,
		     const ActiveSet& set)
{
  // match bcast_e()'s in serve_evaluations_{synch,asynch,peer}
  parallelLib.bcast_e(fn_eval_id);
  MPIPackBuffer send_buffer(lenVarsActSetMessage);
  send_buffer << vars << set;
  parallelLib.bcast_e(send_buffer);
}


/** Invoked by the serve() function in derived Model classes.
    Passes control to serve_evaluations_asynch(),
    serve_evaluations_peer(), or serve_evaluations_synch() according 
    to specified concurrency and self/static scheduler configuration. */
void ApplicationInterface::serve_evaluations()
{
  // This function is only called when message passing, therefore the simple 
  // test below is sufficient.  Would like to use serve_evaluation_synch() in
  // the case where a request for asynch local concurrency is made but it is
  // not utilized (num_jobs <= numEvalServers in *_schedule_evaluations), but 
  // this is not currently detectable by the slave since num_jobs is not known
  // a priori.  serve_evaluation_asynch() still works in this case, but it
  // exposes the run to race conditions from the asynch processing procedure.
  // Thus, it is up to the user to avoid specifying asynch local concurrency
  // when num_jobs is not strictly greater than numEvalServers.

  // test for server 1 within a static schedule: multiProcEvalFlag is implied
  // since evalCommRank 0 is running the iterator/job schedulers
  bool peer_server1 = (!ieDedMasterFlag && evalServerId == 1);

  if (asynchLocalEvalConcurrency > 1) {
    if (peer_server1) serve_evaluations_asynch_peer();
    else              serve_evaluations_asynch();
  }
  else {
    if (peer_server1) serve_evaluations_synch_peer();
    else              serve_evaluations_synch();
  }
}


/** This code is invoked by serve_evaluations() to perform one
    synchronous job at a time on each slave/peer server.  The servers
    receive requests (blocking receive), do local synchronous maps,
    and return results.  This is done continuously until a termination
    signal is received from the master (sent via
    stop_evaluation_servers()). */
void ApplicationInterface::serve_evaluations_synch()
{
  // update class member eval id for usage on iteratorCommRank!=0 processors
  // (Use case: special logic within derived direct interface plug-ins)
  currEvalId = 1;
  MPI_Status status; // holds source, tag, and number received in MPI_Recv
  MPI_Request request = MPI_REQUEST_NULL; // bypass MPI_Wait on first pass
  MPIPackBuffer send_buffer(lenResponseMessage); // prevent dealloc @loop end
  while (currEvalId) {
    MPIUnpackBuffer recv_buffer(lenVarsActSetMessage);
    // blocking receive of x & set
    if (evalCommRank == 0) { // 1-level or local comm. leader in 2-level
      parallelLib.recv_ie(recv_buffer, 0, MPI_ANY_TAG, status);
      currEvalId = status.MPI_TAG;
    }
    if (multiProcEvalFlag) { // multilevel must Bcast x/set over evalComm
      parallelLib.bcast_e(currEvalId);
      if (currEvalId)
        parallelLib.bcast_e(recv_buffer);
    }

    if (currEvalId) { // currEvalId = 0 is the termination signal

      Variables vars; // could slave's Model::currentVariables be 
      // used instead? (would remove need to pass vars flags in MPI buffers)
      ActiveSet set;
      recv_buffer >> vars >> set;

#ifdef MPI_DEBUG
      Cout << "Slave receives vars/set buffer which unpacks to:\n" << vars 
           << "Active set vector = { ";
      set.request_vector().write_annotated(s, false);
      Cout << '}' << std::endl;
#endif // MPI_DEBUG

      Response local_response(set); // special constructor

      // slaves invoke derived_map to avoid repeating overhead of map fn.
      try { derived_map(vars, set, local_response, currEvalId); } // synch local

      catch(int fail_code) { // value of fail_code not currently used.
        //Cerr<< "Slave has caught exception from local derived_map"<<std::endl;
        manage_failure(vars, set, local_response, currEvalId);
      }

      // bypass MPI_Wait the 1st time through, since there has been no Isend
      if (request != MPI_REQUEST_NULL) // MPI_REQUEST_NULL = -1 IBM, 0 elsewhere
        parallelLib.wait(request, status); // Removes need for send_bufs array
        // by assuring that send_buffer is undisturbed prior to receipt by
        // master.  This design should work well for expensive fn. evals., but
        // the master might become overloaded for many inexpensive evals.

      // Return local_response data.  Isend allows execution of next job to
      // start (if master has posted several jobs for this slave -- e.g., a
      // static schedule) prior to master receiving the current results.  This
      // allows the response to be overwritten, but send_buffer overwrite is
      // prevented by MPI_Wait.
      if (evalCommRank == 0) { // 1-level or local comm. leader in 2-level
        send_buffer.reset();
        send_buffer << local_response;
        parallelLib.isend_ie(send_buffer, 0, currEvalId, request);
      }
    }
  }
}


/** This code is invoked by serve_evaluations() to perform a
    synchronous evaluation in coordination with the iteratorCommRank 0
    processor (the iterator) for static schedules.  The bcast()
    matches either the bcast() in synchronous_local_evaluations(),
    which is invoked by static_schedule_evaluations(), or the bcast()
    in map(). */
void ApplicationInterface::serve_evaluations_synch_peer()
{ 
  // update class member eval id for usage on iteratorCommRank!=0 processors
  // (Use case: special logic within derived direct interface plug-ins)
  currEvalId = 1;
  while (currEvalId) {
    parallelLib.bcast_e(currEvalId); // incoming from iterator

    if (currEvalId) { // currEvalId = 0 is the termination signal

      MPIUnpackBuffer recv_buffer(lenVarsActSetMessage);
      parallelLib.bcast_e(recv_buffer); // incoming from iterator

      Variables vars;
      ActiveSet set;
      recv_buffer >> vars >> set;

#ifdef MPI_DEBUG
      Cout << "Peer receives vars/set buffer which unpacks to:\n" << vars 
           << "Active set vector = { ";
      set.request_vector().write_annotated(s, false);
      Cout << '}' << std::endl;
#endif // MPI_DEBUG

      Response local_response(set); // special constructor

      // slaves invoke derived_map to avoid repeating overhead of map fn.
      try { derived_map(vars, set, local_response, currEvalId); } //synch local

      catch(int fail_code) { // value of fail_code not currently used.
        //Cerr<< "Slave has caught exception from local derived_map"<<std::endl;
        manage_failure(vars, set, local_response, currEvalId);
      }
    }
  }
}


/** This code is invoked by serve_evaluations() to perform multiple
    asynchronous jobs on each slave/peer server.  The servers test for
    any incoming jobs, launch any new jobs, process any completed
    jobs, and return any results.  Each of these components is
    nonblocking, although the server loop continues until a
    termination signal is received from the master (sent via
    stop_evaluation_servers()).  In the master-slave case, the master
    maintains the correct number of jobs on each slave.  In the static
    scheduling case, each server is responsible for limiting
    concurrency (since the entire static schedule is sent to the peers
    at start up). */
void ApplicationInterface::serve_evaluations_asynch()
{
  // ---------------------------------------------------------------------------
  // multiprocessor system calls, forks, and threads can't share a communicator,
  // only synchronous direct interfaces can.  Therefore, this asynch job
  // scheduler would not normally need to support a multiprocessor evalComm
  // (ApplicationInterface::init_communicators_checks() normally prevents this
  // in its check for multiproc evalComm and asynchLocalEvalConcurrency > 1).
  // However, direct interface plug-ins can use derived_{synch,synch_nowait} to
  // provide a poor man's batch processing capability, so this function has been
  // extended to allow for multiProcEvalFlag to accommodate this case.
  // ---------------------------------------------------------------------------

  // ----------------------------------------------------------
  // Step 1: block on first message before entering while loops
  // ----------------------------------------------------------
  MPIUnpackBuffer recv_buffer(lenVarsActSetMessage);
  MPI_Status status; // holds MPI_SOURCE, MPI_TAG, & MPI_ERROR
  int fn_eval_id = 1, num_active = 0;
  MPI_Request recv_request = MPI_REQUEST_NULL; // bypass MPI_Test on first pass
  PRPQueue active_prp_queue;
  if (evalCommRank == 0) // 1-level or local comm. leader in 2-level
    parallelLib.recv_ie(recv_buffer, 0, MPI_ANY_TAG, status);

  do { // main loop

    // -------------------------------------------------------------------
    // Step 2: check for additional incoming messages & unpack/execute all 
    //         jobs received
    // -------------------------------------------------------------------
    int mpi_test_flag = 1;
    // num_active < asynchLocalEvalConcurrency check below manages concurrency
    // in the static scheduling case (self_schedule_evaluations handles
    // this from the master side)
    while (mpi_test_flag && fn_eval_id &&
           num_active < asynchLocalEvalConcurrency) {
      // test for completion (Note: MPI_REQUEST_NULL != 0 on IBM)
      if (evalCommRank == 0 && recv_request != MPI_REQUEST_NULL)
	parallelLib.test(recv_request, mpi_test_flag, status);
      if (multiProcEvalFlag)
	parallelLib.bcast_e(mpi_test_flag);
      // if test indicates a completion: unpack, execute, & repost
      if (mpi_test_flag) {

	if (evalCommRank == 0)
	  fn_eval_id = status.MPI_TAG;
	if (multiProcEvalFlag)
	  parallelLib.bcast_e(fn_eval_id);

        if (fn_eval_id) {
	  if (multiProcEvalFlag)
	    parallelLib.bcast_e(recv_buffer);
	  // unpack
          Variables vars; ActiveSet set;
          recv_buffer >> vars >> set;
	  recv_buffer.reset();
	  Response local_response(set); // special constructor
          ParamResponsePair prp(vars, interfaceId, local_response,
				fn_eval_id, false); // shallow copy
          active_prp_queue.insert(prp);
	  // execute
          derived_map_asynch(prp);
          ++num_active;
	  // repost
	  if (evalCommRank == 0)
	    parallelLib.irecv_ie(recv_buffer, 0, MPI_ANY_TAG, recv_request);
	}
      }
    }

    // -----------------------------------------------------------------
    // Step 3: check for any completed jobs and return results to master
    // -----------------------------------------------------------------
    if (num_active > 0) {
      completionSet.clear();
      derived_synch_nowait(active_prp_queue); // rebuilds completionSet
      num_active -= completionSet.size();
      PRPQueueIter q_it;
      for (ISCIter id_iter = completionSet.begin();
	   id_iter != completionSet.end(); ++id_iter) {
        int completed_eval_id = *id_iter;
	q_it = lookup_by_eval_id(active_prp_queue, completed_eval_id);
	if (q_it == active_prp_queue.end()) {
	  Cerr << "Error: failure in queue lookup within ApplicationInterface::"
	       << "serve_evaluations_asynch()." << std::endl;
	  abort_handler(-1);
	}
	else {
	  if (evalCommRank == 0) {
	    // In this case, use a blocking send to avoid having to manage waits
	    // on multiple send buffers (which would be a pain since the number
	    // of send_buffers would vary with completionSet length).
	    MPIPackBuffer send_buffer(lenResponseMessage);
	    send_buffer << q_it->prp_response();
	    parallelLib.send_ie(send_buffer, 0, completed_eval_id);
	  }
	  active_prp_queue.erase(q_it);
	}
      }
    }

  } while (fn_eval_id || num_active > 0);

  //clear_bookkeeping(); // clear any bookkeeping lists in derived classes
}


/** This code is invoked by serve_evaluations() to perform multiple
    asynchronous jobs on multiprocessor slave/peer servers.  It
    matches the multiProcEvalFlag bcasts in
    ApplicationInterface::asynchronous_local_evaluations(). */
void ApplicationInterface::serve_evaluations_asynch_peer()
{
  MPIUnpackBuffer recv_buffer(lenVarsActSetMessage);
  int fn_eval_id = 1, num_jobs;
  size_t num_active = 0, num_completed;
  PRPQueue active_prp_queue;

  parallelLib.bcast_e(num_jobs);
  int num_launch = std::min(asynchLocalEvalConcurrency, num_jobs);

  do { // main loop

    // -------------------------------------------------------------------
    // check for incoming messages & unpack/execute all jobs received
    // -------------------------------------------------------------------
    while (fn_eval_id && num_active < num_launch) {
      parallelLib.bcast_e(fn_eval_id);
      if (fn_eval_id) {
	parallelLib.bcast_e(recv_buffer);
	// unpack
	Variables vars;	ActiveSet set;
	recv_buffer >> vars >> set;
	recv_buffer.reset();
	Response local_response(set); // special constructor
	ParamResponsePair prp(vars, interfaceId, local_response,
			      fn_eval_id, false); // shallow copy
	active_prp_queue.insert(prp);
	// execute
	derived_map_asynch(prp);
	++num_active;
      }
    }

    // -----------------------------------------------------------------
    // check for any completed jobs and return results to master
    // -----------------------------------------------------------------
    if (num_active > 0) {
      completionSet.clear();
      derived_synch_nowait(active_prp_queue); // rebuilds completionSet
      num_completed = completionSet.size();
      if (num_completed == num_active)
	{ num_active = 0; active_prp_queue.clear(); }
      else {
	num_active -= num_completed; //num_jobs_remaining -= num_completed;
	PRPQueueIter q_it; ISCIter id_it;
	for (id_it=completionSet.begin(); id_it!=completionSet.end(); ++id_it) {
	  q_it = lookup_by_eval_id(active_prp_queue, *id_it);
	  if (q_it == active_prp_queue.end()) {
	    Cerr << "Error: failure in queue lookup within ApplicationInterface"
		 << "::serve_evaluations_asynch_peer()." << std::endl;
	    abort_handler(-1);
	  }
	  else
	    active_prp_queue.erase(q_it);
	}
      }
    }

  } while (fn_eval_id || num_active > 0);

  //clear_bookkeeping(); // clear any bookkeeping lists in derived classes
}


/** This code is executed on the iteratorComm rank 0 processor when
    iteration on a particular model is complete.  It sends a
    termination signal (tag = 0 instead of a valid fn_eval_id) to each
    of the slave analysis servers.  NOTE: This function is called from
    the Strategy layer even when in serial mode.  Therefore, use
    iteratorCommSize to provide appropriate fall through behavior. */
void ApplicationInterface::stop_evaluation_servers()
{ 
  if (iteratorCommSize > 1) {
    if (!ieDedMasterFlag) {
      Cout << "Peer 1 stopping" << std::endl;
      if (multiProcEvalFlag) { // stop serve_peer procs
        int fn_eval_id = 0;
        parallelLib.bcast_e(fn_eval_id);
      }
    }
    MPIPackBuffer send_buffer(0); // empty buffer
    MPI_Request send_request;
    int term_tag = 0; // triggers termination
    int end = (ieDedMasterFlag) ? numEvalServers : numEvalServers-1;
    for (int i=0; i<end; ++i) { // stop serve_evaluation_synch/asynch procs
      int server_id = i + 1;
      if (ieDedMasterFlag)
        Cout << "Master stopping server " << server_id << std::endl;
      else
        Cout << "Peer " << server_id+1 << " stopping"<< std::endl;
      // nonblocking sends: master posts all terminate messages without waiting
      // for completion.  Bcast cannot be used since all procs must call it and
      // slaves are using Recv/Irecv in serve_evaluation_synch/asynch.
      parallelLib.isend_ie(send_buffer, server_id, term_tag, send_request);
      parallelLib.free(send_request); // no test/wait on send_request
    }
  }
}


// --------------------------------------------------
// Schedulers for concurrent analyses within fn evals
// --------------------------------------------------
/** This code is called from derived classes to provide the master
    portion of a master-slave algorithm for the dynamic
    self-scheduling of analyses among slave servers.  It is patterned
    after self_schedule_evaluations(). It performs no analyses locally
    and matches either serve_analyses_synch() or
    serve_analyses_asynch() on the slave servers, depending on the
    value of asynchLocalAnalysisConcurrency.  Self-scheduling approach
    assigns jobs in 2 passes.  The 1st pass gives each server the same
    number of jobs (equal to asynchLocalAnalysisConcurrency).  The 2nd
    pass assigns the remaining jobs to slave servers as previous jobs
    are completed.  Single- and multilevel parallel use intra- and
    inter-communicators, respectively, for send/receive.  Specific
    syntax is encapsulated within ParallelLibrary. */
void ApplicationInterface::self_schedule_analyses()
{
  int capacity  = (asynchLocalAnalysisConcurrency) ?
                   asynchLocalAnalysisConcurrency*numAnalysisServers : 
                   numAnalysisServers;
  int num_sends = (capacity < numAnalysisDrivers) ? 
                   capacity : numAnalysisDrivers;
#ifdef MPI_DEBUG
  Cout << "First pass: assigning " << num_sends << " analyses among " 
       << numAnalysisServers << " servers\n";
#endif // MPI_DEBUG
  MPI_Request  send_request; // only 1 needed since no test/wait on sends
  int*         rtn_codes     = new int [num_sends];
  MPI_Request* recv_requests = new MPI_Request [num_sends];
  int i, server_id, analysis_id;
  for (i=0; i<num_sends; ++i) { // send data & post recvs for 1st pass
    server_id = i%numAnalysisServers + 1; // from 1 to numAnalysisServers
    analysis_id = i+1;
#ifdef MPI_DEBUG
    Cout << "Master assigning analysis " << analysis_id << " to server " 
         << server_id << '\n';
#endif // MPI_DEBUG
    // pre-post receives.  
    parallelLib.irecv_ea(rtn_codes[i], server_id, analysis_id,
                         recv_requests[i]);
    parallelLib.isend_ea(analysis_id, server_id, analysis_id, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }
  if (num_sends < numAnalysisDrivers) { // self-schedule remaining analyses
#ifdef MPI_DEBUG
    Cout << "Second pass: self-scheduling " << numAnalysisDrivers-num_sends 
         << " remaining analyses\n";
#endif // MPI_DEBUG
    int send_cntr = num_sends, recv_cntr = 0, out_count;
    MPI_Status* status_array = new MPI_Status [num_sends];
    int*        index_array  = new int [num_sends];
    while (recv_cntr < numAnalysisDrivers) {
#ifdef MPI_DEBUG
      Cout << "Waiting on completed analyses" << std::endl;
#endif // MPI_DEBUG
      parallelLib.waitsome(num_sends, recv_requests, out_count, index_array, 
			   status_array);
      recv_cntr += out_count;
      for (i=0; i<out_count; ++i) {
        int index = index_array[i]; // index of recv_request that completed
        server_id = index%numAnalysisServers + 1;// from 1 to numAnalysisServers
#ifdef MPI_DEBUG
        Cout << "analysis " << status_array[i].MPI_TAG 
             <<" has returned from slave server " << server_id << '\n';
#endif // MPI_DEBUG
        if (send_cntr < numAnalysisDrivers) {
          analysis_id = send_cntr+1;
#ifdef MPI_DEBUG
          Cout << "Master assigning analysis " << analysis_id << " to server "
               << server_id << '\n';
#endif // MPI_DEBUG
	  // pre-post receives
          parallelLib.irecv_ea(rtn_codes[index], server_id, analysis_id, 
                               recv_requests[index]);
          parallelLib.isend_ea(analysis_id, server_id, analysis_id, 
                               send_request);
          parallelLib.free(send_request); // no test/wait on send_request
          ++send_cntr;
        }
      }
    }
    delete [] status_array;
    delete [] index_array;
  }
  else { // all analyses assigned in first pass
#ifdef MPI_DEBUG
    Cout << "Waiting on all analyses" << std::endl;
#endif // MPI_DEBUG
    parallelLib.waitall(numAnalysisDrivers, recv_requests);
  }
  delete [] rtn_codes;
  delete [] recv_requests;

  // Unlike ApplicationInterface::self_schedule_message_passing (which 
  // terminates servers only when the iterator/model is complete), terminate 
  // servers now so that they can return from derived_map to the higher level.
  analysis_id = 0; // termination flag for servers
  for (i=0; i<numAnalysisServers; ++i) {
    parallelLib.isend_ea(analysis_id, i+1, 0, send_request);
    parallelLib.free(send_request); // no test/wait on send_request
  }
}


/** This code is called from derived classes to run synchronous
    analyses on slave processors.  The slaves receive requests
    (blocking receive), do local derived_map_ac's, and return codes.
    This is done continuously until a termination signal is received
    from the master.  It is patterned after
    serve_evaluations_synch(). */
void ApplicationInterface::serve_analyses_synch()
{
  int analysis_id = 1;
  MPI_Status status; // holds source, tag, and number received in MPI_Recv
  MPI_Request request = MPI_REQUEST_NULL; // bypass MPI_Wait on first pass
  while (analysis_id) {
    // blocking receive of analysis id
    if (analysisCommRank == 0) // 1-level or comm. leader in 2-level (DirectFn)
      parallelLib.recv_ea(analysis_id, 0, MPI_ANY_TAG, status);
    // NOTE: could get analysis_id from status.MPI_TAG & receive something
    //       else useful in 1st integer slot.  One possibility: tag with 
    //       (fn_eval_id-1)*numAnalysisDrivers+analysis_id to prevent any 
    //       possible tag overlap with message traffic at other parallelism 
    //       levels.  However, I think the use of communicators prevent these
    //       types of errors.
    if (multiProcAnalysisFlag) // must Bcast over analysisComm
      parallelLib.bcast_a(analysis_id);

    if (analysis_id) { // analysis_id = 0 is the termination signal

      int rtn_code = derived_synchronous_local_analysis(analysis_id);

      if (request != MPI_REQUEST_NULL) // MPI_REQUEST_NULL = -1 IBM, 0 elsewhere
        parallelLib.wait(request, status); // block until prev isend completes

      // Return the simulation failure code and tag with analysis_id
      // NOTE: the rtn_code is not currently used for anything on the master 
      //       since failure capturing exceptions are captured in 
      //       read_results_files (sys call, fork) or derived_map_ac (direct).
      //       As for recv_ea above, it could be replaced with something else.
      if (analysisCommRank == 0)// 1-level or comm. leader in 2-level (DirectFn)
        parallelLib.isend_ea(rtn_code, 0, analysis_id, request);
    }
  }
}


// -----------------------------------------
// Routines for managing simulation failures
// -----------------------------------------
void ApplicationInterface::
manage_failure(const Variables& vars, const ActiveSet& set, Response& response, 
	       int failed_eval_id)
{
  // SysCall/Fork application interface exception handling:
  // When a simulation failure is detected w/i Response::read(istream), 
  // an int exception is thrown which is first caught by catch(int fail_code) 
  // within derived_map or derived_synch, depending on whether the simulation
  // was synch or asynch.  In the case of derived_map, it rethrows the
  // exception to an outer catch in map or serve which then invokes 
  // manage_failure.  In the case of derived_synch, it invokes manage_failure
  // directly.  manage_failure can then catch subsequent exceptions resulting
  // from the try blocks below.

  // DirectFn application interface exception handling:
  // The DirectFn case is simpler than SysCall since (1) synch case: the 
  // exception can be thrown directly from derived_map to map or serve which 
  // then invokes manage_failure (which then catches additional exceptions 
  // thrown directly from derived_map), or (2) asynch case: derived_synch
  // throws no exceptions, but invokes manage_failure directly.

  if (failAction == "retry") {
    //retry(vars, set, response, failRetryLimit);
    int retries = 0;
    bool fail_flag = 1; // allow 1st pass through the while test
    while (fail_flag) {
      fail_flag = 0; // reset each time prior to derived_map
      ++retries;
      Cout << "Failure captured: retry attempt number " << retries << ".\n";
      try { derived_map(vars, set, response, failed_eval_id); }
      catch(int fail_code) { 
        //Cout << "Caught int in manage_failure" << std::endl;
        fail_flag = 1;
	if (retries >= failRetryLimit) {
	  Cerr << "Retry limit exceeded.  Aborting..." << std::endl;
          abort_handler(-1);
	}
      }
    }
  }
  else if (failAction == "recover") {
    Cout << "Failure captured: recovering with specified function values.\n";
    if (failRecoveryFnVals.length() != response.num_functions() ) {
      Cerr << "Error: length of recovery function values specification\n"
           << "       must equal the total number of functions." << std::endl;
      abort_handler(-1);
    }
    // reset response to avoid bleeding over of derivatives from previous eval.
    response.reset();
    // Set specified recovery function values
    response.function_values(failRecoveryFnVals);
  }
  else if (failAction == "continuation") {
    // Compute closest source pt. for continuation from extern data_pairs.
    ParamResponsePair source_pair;
    // THIS CODE BLOCK IS A PLACEHOLDER AND IS NOT YET OPERATIONAL
    if (iteratorCommRank) { // if other than master
      // Get source pt. for continuation.  Master calls get_source_point for 
      // slave (since it has access to data_pairs) and returns result.
      MPIPackBuffer send_buffer(lenVarsMessage);
      send_buffer << vars;
      // master must receive failure message w/i *_schedule_evaluations 
      // after MPI_Waitany or MPI_Waitall.  Use tag < 0 ?
      parallelLib.send_ie(send_buffer, 0, failed_eval_id);
      MPIUnpackBuffer recv_buffer(lenPRPairMessage);
      MPI_Status recv_status;
      parallelLib.recv_ie(recv_buffer, 0, failed_eval_id, recv_status);
      recv_buffer >> source_pair;
    }
    else
      source_pair = get_source_pair(vars); // also the serial case

    Cout << "\nFailure captured: halving interval and retrying." << std::endl;

    // Now that source pt. is available, call the continuation algorithm.  
    // Mimic retry case in use of failed_eval_id to manage file names.

    continuation(vars, set, response, source_pair, failed_eval_id);

  }
  else { // default is abort
    Cerr << "Failure captured: aborting..." << std::endl;
    abort_handler(-1);
  }
}


const ParamResponsePair& 
ApplicationInterface::get_source_pair(const Variables& target_vars)
{
  if (data_pairs.size() == 0) {
    Cerr << "Failure captured: No points available, aborting" << std::endl;
    abort_handler(-1);
  }

  // TO DO: should check both continuous_variables and discrete_variables
  const RealVector& xc_target = target_vars.continuous_variables();

  size_t i, num_vars = xc_target.length();
  Real best_sos = DBL_MAX;

  // TO DO: Need to check for same interfaceId as well.  Currently, this is 
  // part of Response -> need to add to Interface.
  PRPCacheCIter prp_iter, prp_end_iter = data_pairs.end(), best_iter;
  for (prp_iter  = data_pairs.begin(); prp_iter != prp_end_iter; ++prp_iter) {
    //if (interfaceId == prp_iter->interface_id()) {
      const RealVector& xc_source 
	= prp_iter->prp_parameters().continuous_variables();
      Real sum_of_squares = 0.;
      for (i=0; i<num_vars; ++i)
        sum_of_squares += std::pow( xc_source[i] - xc_target[i], 2);
      if (prp_iter == data_pairs.begin() || sum_of_squares < best_sos) {
        best_iter = prp_iter;
        best_sos  = sum_of_squares;
      }
    //}
  }

  // Desired implementation:
  //return *best_iter;

  // For now, this asks the least of the simulation management:
  --prp_iter; // last PRPair is one back from end()
  return *prp_iter;
  // Both the variables and the response (in a PRPair) must be returned since 
  // the variables alone are not enough to enact the continuation.  The 
  // corresponding response must be used as the simulator initial guess.
}


void ApplicationInterface::
continuation(const Variables& target_vars, const ActiveSet& set, 
	     Response& response, const ParamResponsePair& source_pair, 
	     int failed_eval_id)
{
  // TO DO: should use both continuous_variables and discrete_variables
  const Variables&     source_vars = source_pair.prp_parameters();
  const RealVector& source_pt = source_vars.continuous_variables();
  const RealVector& target_pt = target_vars.continuous_variables();

  // copy() required to avoid modifying variables in data_pairs entry (standard
  // operator= behavior would have source_vars sharing a variablesRep with the
  // identified data_pairs entry).
  Variables current_vars = source_vars.copy();

  size_t i, num_cv = source_pt.length();
  short  failures = 1, MAX_FAILURES = 10;
  bool   target_reached = false;
  float  EPS = 1.0e-10;

  RealVector current_pt(num_cv, 0.), delta(num_cv, 0.);
  for (i=0; i<num_cv; ++i) {
    // Define delta to be half the distance to the target
    delta[i] = (target_pt[i] - source_pt[i])/2.;
    current_pt[i] = source_pt[i] + delta[i];
  }

  while (!target_reached) {

    // Perform the intermediate function evaluation
    current_vars.continuous_variables(current_pt);

    // TO DO: the simulation must be seeded with the response corresponding to
    // the (current) source information.  Thus, the current implementation 
    // only works for best_iter from get_source_point = last evaluation.
    //write_continuation_seed_file(source_response);

    bool fail_flag = 0;
    try { derived_map(current_vars, set, response, failed_eval_id); }
    catch(int fail_code) { 
      fail_flag = 1;
    }

    if (fail_flag) {
      ++failures;
      Cout << "\nFunction evaluation failed. Interval halving." << std::endl;
      if (failures > MAX_FAILURES) {
	Cerr << "\n\nInterval halving limit exceeded in continuation: "
	     << "aborting..." << std::endl;
	abort_handler(-1);
      }

      // take a half-step back from the failed current point (and don't update
      // source_vars -> keep the same seed file)
      for (i=0; i<num_cv; ++i) {
	delta[i] /= 2.0;
	current_pt[i] -= delta[i];
      }
    }
    else {
      Cout << "\nFunction evaluation succeeded.\nContinuing with current step "
	   << "size." << std::endl;

      // Possibly append new continuation evals. to data_pairs list (?)

      if (current_pt == target_pt) // operator== in data_types.C
        target_reached = true;
      else {
        for (i=0; i<num_cv; ++i) {
          current_pt[i] += delta[i];
          if (std::fabs(1.0 - current_pt[i]/target_pt[i]) < EPS)
            current_pt[i] = target_pt[i]; // make sure operator== is within
                                          // DBL_EPSILON tolerance
        }
        // TO DO: update source response for seed file
      }
    }
  }
  Cout << "Finished with continuation." << std::endl;
}


void ApplicationInterface::common_input_filtering(const Variables& vars)
{ } // empty for now


void ApplicationInterface::common_output_filtering(Response& response)
{ } // empty for now

} // namespace Dakota
