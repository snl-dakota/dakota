/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SequentialHybridStrategy
//- Description: Implementation code for the SequentialHybridStrategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "SequentialHybridStrategy.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "data_io.h"

static const char rcsId[]="@(#) $Id: SequentialHybridStrategy.C 6972 2010-09-17 22:18:50Z briadam $";


using namespace std;

namespace Dakota {

SequentialHybridStrategy::SequentialHybridStrategy(ProblemDescDB& problem_db):
  HybridStrategy(problem_db), 
  hybridType(problem_db.get_string("strategy.hybrid.type"))
{
  if (worldRank == 0)
    cout << "Constructing Sequential Hybrid Optimizer Strategy...\n";

  methodList = problem_db.get_dsa("strategy.hybrid.method_list");
  numIterators = methodList.size();
  if (!numIterators) { // verify at least one method in list
    cerr << "Error: method_list must have a least one entry." << endl;
    abort_handler(-1);
  }
  if (hybridType.ends("_adaptive") && worldRank == 0) {
    progressThreshold
      = problem_db.get_real("strategy.hybrid.progress_threshold");
    if (progressThreshold > 1.) {
      cerr << "Warning: progress_threshold should be <= 1.  Setting to 1.\n";
      progressThreshold = 1.;
    }
    else if (progressThreshold < 0.) {
      cerr << "Warning: progress_threshold should be >= 0.  Setting to 0.\n";
      progressThreshold = 0.;
    }
  }

  // define maxConcurrency without access to selectedIterators
  // (init_iterator_parallelism() requires maxConcurrency and
  // init_iterator_parallelism() must precede allocate_methods()).
  maxConcurrency = 1;
  size_t i, sizet_max = std::numeric_limits<std::size_t>::max();
  problem_db.set_db_list_nodes(methodList[0]);
  String curr_method = problem_db.get_string("method.algorithm");
  for (i=0; i<numIterators; ++i) {
    // current method data
    size_t num_curr_final = problem_db.get_sizet("method.final_solutions");
    if (!num_curr_final) // manually replicate iterator-specific defaults
      num_curr_final = (curr_method == "moga") ? INT_MAX : 1;
    else if (num_curr_final > INT_MAX)
      num_curr_final = INT_MAX; // maxConcurrency is int

    if (i < numIterators-1) {
      // look ahead method data
      problem_db.set_db_list_nodes(methodList[i+1]);
      const String& next_method = problem_db.get_string("method.algorithm");
      // manually replicate accepts_multiple_points() support
      bool next_single = (next_method != "moga" && next_method != "soga");
      if (next_single && num_curr_final > maxConcurrency)
	maxConcurrency = num_curr_final;

      // updates for next iteration
      curr_method = next_method;
    }

    /* Cannot use this logic since selectedIterators not available
    bool next_single = (i < numIterators-1 &&
			!selectedIterators[i+1].accepts_multiple_points());
    if (next_single) {
      size_t num_curr_final = selectedIterators[i].num_final_solutions();
      if (num_curr_final > maxConcurrency) 
	maxConcurrency = num_curr_final;
    }
    */
  }

  init_iterator_parallelism();
  // Adaptive hybrid does not support iterator concurrency --> verify settings.
  if ( ( stratIterDedMaster || numIteratorServers > 1 ) && 
       hybridType.ends("_adaptive") ) {
    cerr << "Error: Adaptive Sequential Hybrid Strategy does not support "
	 << "concurrent iterator parallelism." << endl;
    abort_handler(-1);
  }
  allocate_methods();

  // size prpResults: all iterator masters bookkeep on the full results list,
  // even if only some entries are defined locally (prior to All-Reduce at end
  // of each cycle in run_sequential())
  if (iteratorCommRank == 0) {
    size_t i, num_final_solns, max_final_solns = 1,
      sizet_max = std::numeric_limits<std::size_t>::max();
    bool reassign = false;
    for (i=0; i<numIterators; ++i) {
      num_final_solns = selectedIterators[i].num_final_solutions();
      if (num_final_solns == sizet_max) { // JEGA default
	// pick a reasonable heuristic now that partitioning has occurred
	num_final_solns = numIteratorServers;
	reassign = true;
      }
      if (num_final_solns > max_final_solns)
	max_final_solns = num_final_solns;
    }
    prpResults.resize(max_final_solns);
    // override any sizet_max instances
    if (reassign)
      for (i=0; i<numIterators; ++i)
	if (selectedIterators[i].num_final_solutions() == sizet_max)
	  selectedIterators[i].num_final_solutions(max_final_solns);
  }
}


SequentialHybridStrategy::~SequentialHybridStrategy()
{
  // Virtual destructor handles referenceCount at Strategy level.

  deallocate_methods();
}


void SequentialHybridStrategy::run_strategy()
{
  if (hybridType.ends("_adaptive"))
    run_sequential_adaptive();
  else
    run_sequential();
}


/** In the sequential nonadaptive case, there is no interference with
    the iterators.  Each runs until its own convergence criteria is
    satisfied.  Status: fully operational. */
void SequentialHybridStrategy::run_sequential()
{
  //size_t iterator_capacity = 1, max_instances = 10;
  Cout << "MaxConcurrency " << maxConcurrency << '\n';

  for (seqCount=0; seqCount<numIterators; seqCount++) {

    // each of these is safe for all processors
    Iterator& curr_iterator      = selectedIterators[seqCount];
    Model&    curr_model         = userDefinedModels[seqCount];
    bool      curr_accepts_multi = curr_iterator.accepts_multiple_points();
    bool      curr_returns_multi = curr_iterator.returns_multiple_points();
 
    if (worldRank == 0) {
      cout << "\n>>>>> Running Sequential Hybrid Optimizer Strategy with "
	   << "iterator " << methodList[seqCount] << ".\n";
      // set up plots and tabular data file
      curr_iterator.initialize_graphics(graph2DFlag, tabularDataFlag,
					tabularDataFile);
    }

    // -------------------------------------------------------------
    // Define total number of runs for this iterator in the sequence
    // -------------------------------------------------------------
    // > run 1st iterator as is, using single default starting pt
    // > subsequent iteration may involve multipoint data flow
    // > In the future, we may support concurrent multipoint iterators, but
    //   prior to additional specification data, we either have a single
    //   multipoint iterator or concurrent single-point iterators.
    if (seqCount == 0)
      numIteratorJobs = 1;
    else if (stratIterDedMaster) {
      // send curr_accepts_multi from 1st iterator master to strategy master
      if (iteratorCommRank == 0 && iteratorServerId == 1) {
	int multi_flag = (int)curr_accepts_multi; // bool -> int
        parallelLib.send_si(multi_flag, 0, 0);
      }
      else if (worldRank == 0) {
	int multi_flag; MPI_Status status;
	parallelLib.recv_si(multi_flag, 1, 0, status);
	curr_accepts_multi = (bool)multi_flag; // int -> bool
	numIteratorJobs = (curr_accepts_multi) ? 1 : prpResults.size();
      }
    }
    else { // static scheduling
      if (iteratorCommRank == 0)
	numIteratorJobs = (curr_accepts_multi) ? 1 : prpResults.size();
      // bcast numIteratorJobs over iteratorComm
      if (iteratorCommSize > 1)
	parallelLib.bcast_i(numIteratorJobs);
    }

    // -----------------------------------------
    // Define buffer lengths for message passing
    // -----------------------------------------
    if (stratIterMessagePass && iteratorCommRank == 0) {
      // define paramsMsgLen
      if (stratIterDedMaster) {
	MPIPackBuffer params_buffer;
	pack_parameters_buffer(params_buffer, 0);
	paramsMsgLen = params_buffer.size();
      }
      // define resultsMsgLen
      MPIPackBuffer results_buffer;
      // pack_results_buffer() is not reliable for several reasons:
      // > for seqCount == 0, prpResults contains empty envelopes
      // > for seqCount >= 1, the previous state of prpResults may not
      //   accurately reflect the future state due to the presence of some
      //   multi-point iterators which do not define the results array.
      //pack_results_buffer(results_buffer, 0);
      // The following may be conservative in some cases (e.g., if the results
      // arrays will be empty), but should be reliable.
      ParamResponsePair prp_star(curr_iterator.variables_results(),
        curr_model.interface_id(), curr_iterator.response_results()); // shallow
      size_t start_index, job_size;
      partition_results(0, start_index, job_size);
      results_buffer << job_size;
      for (size_t i=0; i<job_size; i++)
	results_buffer << prp_star;
      resultsMsgLen = results_buffer.size();
    }

    // ---------------------------------------------------
    // Schedule the runs for this iterator in the sequence
    // ---------------------------------------------------
    schedule_iterators(curr_iterator, curr_model);

    // ---------------------------------
    // Post-process the iterator results
    // ---------------------------------
    // migrate results among procs as required for parallel scheduling, e.g.,
    // from multiple single-point iterators to a single multi-point iterator
    // > for dedicated master self-scheduling, all results data resides on the
    //   dedicated master and no additional migration is required.
    // > for peer partition static scheduling, the full prpResults array needs
    //   to be propagated back to peers 2 though n (like an All-Reduce, except
    //   that Strategy::static_schedule_iterators() enforces the reduction to
    //   peer 1 and the code below enforces repropagation from 1 to 2-n).
    if ( !stratIterDedMaster    && iteratorCommRank == 0 &&
	 numIteratorServers > 1 && seqCount+1 < numIterators) {
      if (worldRank == 0) { // send complete list
	MPIPackBuffer send_buffer;
	send_buffer << prpResults;
	int buffer_len = send_buffer.size();
	parallelLib.bcast_si(buffer_len);
	parallelLib.bcast_si(send_buffer);
      }
      else { // replace partial list
	int buffer_len;
	parallelLib.bcast_si(buffer_len);
	MPIUnpackBuffer recv_buffer(buffer_len);
	parallelLib.bcast_si(recv_buffer);
	recv_buffer >> prpResults;
      }
    }
  }

  // Output final results
  if (worldRank == 0) {
    cout << "\n<<<<< Sequential Hybrid Optimizer Strategy completed.\n";
    // provide a final summary in cases where the default iterator output
    // is insufficient
    if (stratIterDedMaster || numIteratorServers > 1) {// || numIteratorJobs > 1
      size_t num_instances = prpResults.size();
      cout << "\n<<<<< Sequential hybrid generated " << num_instances
	   << " final solution sets:\n";
      for (size_t i=0; i<num_instances; ++i) {
	const ParamResponsePair&  prp_i = prpResults[i];
	const Variables&         vars_i = prp_i.prp_parameters();
	const Response&          resp_i = prp_i.prp_response();
	if (!vars_i.is_null())
	  cout << "<<<<< Best parameters          (set " << i+1 << ") =\n"
	       << vars_i;
	if (!resp_i.is_null()) {
	  cout << "<<<<< Best response functions  (set " << i+1 << ") =\n";
	  write_data(cout, resp_i.function_values());
        }
      }
    }
  }
}


/** In the sequential adaptive case, there is interference with the
    iterators through the use of the ++ overloaded operator.  iterator++ runs
    the iterator for one cycle, after which a progress_metric is computed.
    This progress metric is used to dictate method switching instead of
    each iterator's internal convergence criteria.  Status: incomplete. */
void SequentialHybridStrategy::run_sequential_adaptive()
{
  // NOTE 1: The case where the iterator's internal convergence criteria are 
  // satisfied BEFORE the progress_metric must be either handled or prevented.

  // NOTE 2: Parallel iterator scheduling is not currently supported (and this
  // code will fail if non-default iterator servers or scheduling is specified).

  progressMetric = 1.0;
  for (seqCount=0; seqCount<numIterators; seqCount++) {

    if (worldRank == 0) {
      cout << "\n>>>>> Running adaptive Sequential Hybrid Optimizer Strategy "
	   << "with iterator " << methodList[seqCount] << endl;

      // set up plots and tabular data file
      selectedIterators[seqCount].initialize_graphics(graph2DFlag,
        tabularDataFlag, tabularDataFile);

      selectedIterators[seqCount].initialize_run();
      while (progressMetric >= progressThreshold) {
        //selectedIterators[seqCount]++;
        const Response& resp_star
	  = selectedIterators[seqCount].response_results();
        //progressMetric = compute_progress(resp_star);
      }
      selectedIterators[seqCount].finalize_run();
      cout << "\n<<<<< Iterator " << methodList[seqCount]
	   << " completed.  Progress metric has fallen below threshold.\n";

      // Set the starting point for the next iterator.
      if (seqCount+1 < numIterators) {// prevent index out of range on last pass
        // Get best pt. from completed iteration.
        Variables vars_star = selectedIterators[seqCount].variables_results();
        // Set best pt. as starting point for subsequent iterator
        userDefinedModels[seqCount+1].active_variables(vars_star);
      }

      // Send the termination message to the servers for this iterator/model
      userDefinedModels[seqCount].stop_servers();
    }
    else
      run_iterator(selectedIterators[seqCount], userDefinedModels[seqCount]);
  }

  // Output interesting iterator statistics/progress metrics...
  if (worldRank == 0)
    cout << "\n<<<<< adaptive Sequential Hybrid Optimizer Strategy completed."
	 << endl;
}

} // namespace Dakota
