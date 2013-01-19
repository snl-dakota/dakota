/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SequentialHybridStrategy
//- Description: Implementation code for the SequentialHybridStrategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "SequentialHybridStrategy.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "data_io.hpp"

static const char rcsId[]="@(#) $Id: SequentialHybridStrategy.cpp 6972 2010-09-17 22:18:50Z briadam $";


namespace Dakota {

SequentialHybridStrategy::SequentialHybridStrategy(ProblemDescDB& problem_db):
  HybridStrategy(problem_db), 
  hybridType(problem_db.get_string("strategy.hybrid.type"))
{
  if (worldRank == 0)
    Cout << "Constructing Sequential Hybrid Optimizer Strategy...\n";

  methodList = problem_db.get_sa("strategy.hybrid.method_list");
  numIterators = methodList.size();
  if (!numIterators) { // verify at least one method in list
    Cerr << "Error: method_list must have a least one entry." << std::endl;
    abort_handler(-1);
  }
  if (hybridType.ends("_adaptive") && worldRank == 0) {
    progressThreshold
      = problem_db.get_real("strategy.hybrid.progress_threshold");
    if (progressThreshold > 1.) {
      Cerr << "Warning: progress_threshold should be <= 1.  "
		<< "Setting to 1.\n";
      progressThreshold = 1.;
    }
    else if (progressThreshold < 0.) {
      Cerr << "Warning: progress_threshold should be >= 0.  "
		<< "Setting to 0.\n";
      progressThreshold = 0.;
    }
  }

  // define maxConcurrency without access to selectedIterators
  // (init_iterator_parallelism() requires maxConcurrency and
  // init_iterator_parallelism() must precede allocate_methods()).
  maxConcurrency = 1;
  size_t i, curr_final, next_concurrency = 1, sizet_max_replace = 0;
  problem_db.set_db_list_nodes(methodList[0]);
  String curr_method = problem_db.get_string("method.algorithm");
  for (i=0; i<numIterators; ++i) {
    // current method data
    curr_final = problem_db.get_sizet("method.final_solutions");
    // manually replicate iterator-specific defaults, except replace
    // sizet_max with something more useful for concurrency estimation
    if (!curr_final) {
      if (curr_method == "moga")
	curr_final = sizet_max_replace
	  = problem_db.get_int("method.population_size");
      else
	curr_final = 1;
    }

    if (i < numIterators-1) {
      // look ahead method data
      problem_db.set_db_list_nodes(methodList[i+1]);
      const String& next_method = problem_db.get_string("method.algorithm");
      // manually replicate accepts_multiple_points() support
      bool next_multi_pt = (next_method == "moga" || next_method == "soga");
      // allow for multiplicative point growth
      if (next_multi_pt) next_concurrency  = 1;
      else               next_concurrency *= curr_final;
      if (next_concurrency > maxConcurrency)
	maxConcurrency = next_concurrency;
      // updates for next iteration
      curr_method = next_method;
    }
  }
  Cout << "maxConcurrency = " << maxConcurrency << '\n';

  init_iterator_parallelism();
  // verify settings: adaptive hybrid does not support iterator concurrency
  if ( ( stratIterDedMaster || numIteratorServers > 1 ) && 
       hybridType.ends("_adaptive") ) {
    Cerr << "Error: adaptive Sequential Hybrid Strategy does not support "
	      << "concurrent iterator parallelism." << std::endl;
    abort_handler(-1);
  }
  allocate_methods();

  // now that parallel paritioning and iterator allocation has occurred,
  // manage acceptable values for Iterator::numFinalSolutions (needed for
  // resultsMsgLen estimation in run function)
  if (iteratorCommRank == 0 && sizet_max_replace) {
    size_t sizet_max = std::numeric_limits<size_t>::max();
    for (i=0; i<numIterators; ++i)
      if (selectedIterators[i].num_final_solutions() == sizet_max)
	selectedIterators[i].num_final_solutions(sizet_max_replace);
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
  for (seqCount=0; seqCount<numIterators; seqCount++) {

    // each of these is safe for all processors
    Iterator& curr_iterator      = selectedIterators[seqCount];
    Model&    curr_model         = userDefinedModels[seqCount];
    bool      curr_accepts_multi = curr_iterator.accepts_multiple_points();
    bool      curr_returns_multi = curr_iterator.returns_multiple_points();
 
    if (worldRank == 0) {
      Cout << "\n>>>>> Running Sequential Hybrid Optimizer Strategy with "
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
    if (seqCount == 0) // initialize numIteratorJobs
      numIteratorJobs = 1;
    else {
      // update numIteratorJobs
      if (stratIterDedMaster) {
	// send curr_accepts_multi from 1st iterator master to strategy master
	if (iteratorCommRank == 0 && iteratorServerId == 1) {
	  int multi_flag = (int)curr_accepts_multi; // bool -> int
	  parallelLib.send_si(multi_flag, 0, 0);
	}
	else if (worldRank == 0) {
	  int multi_flag; MPI_Status status;
	  parallelLib.recv_si(multi_flag, 1, 0, status);
	  curr_accepts_multi = (bool)multi_flag; // int -> bool
	  numIteratorJobs = (curr_accepts_multi) ? 1 : parameterSets.size();
	}
      }
      else { // static scheduling
	if (iteratorCommRank == 0)
	  numIteratorJobs = (curr_accepts_multi) ? 1 : parameterSets.size();
	// bcast numIteratorJobs over iteratorComm
	if (iteratorCommSize > 1)
	  parallelLib.bcast_i(numIteratorJobs);
      }
    }
    // --------------------------
    // size prpResults (2D array)
    // --------------------------
    // The total aggregated set of results:
    // > can grow if multiple iterator jobs return multiple points or if
    //   single instance returns more than used for initialization
    // > can only shrink in the case where single instance returns fewer
    //   than used for initialization
    if (iteratorCommRank == 0)
      prpResults.resize(numIteratorJobs);

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
      // Note: max size_t removed from Iterator::numFinalSolutions in ctor
      size_t prp_return_size = curr_iterator.num_final_solutions();
      results_buffer << prp_return_size;
      for (size_t i=0; i<prp_return_size; ++i)
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
    // convert prpResults to parameterSets for next iteration
    if (iteratorCommRank == 0 && seqCount+1 < numIterators) {
      size_t i, j, num_param_sets = 0, cntr = 0, num_prp_i;
      for (i=0; i<numIteratorJobs; ++i)
	num_param_sets += prpResults[i].size();
      parameterSets.resize(num_param_sets);
      for (i=0; i<numIteratorJobs; ++i) {
	const PRPArray& prp_results_i = prpResults[i];
	num_prp_i = prp_results_i.size();
	for (j=0; j<num_prp_i; ++j, ++cntr)
	  parameterSets[cntr] = prp_results_i[j].prp_parameters();
      }
      // migrate results among procs as required for parallel scheduling, e.g.,
      // from multiple single-point iterators to a single multi-point iterator
      // > for dedicated master self-scheduling, all results data resides on
      //   the dedicated master and no additional migration is required.
      // > for peer partition static scheduling, the full parameterSets array
      //   needs to be propagated back to peers 2 though n (like an All-Reduce,
      //   except that Strategy::static_schedule_iterators() enforces reduction
      //   to peer 1 and the code below enforces repropagation from 1 to 2-n).
      if ( !stratIterDedMaster && numIteratorServers > 1) {
	if (worldRank == 0) { // send complete list
	  MPIPackBuffer send_buffer;
	  send_buffer << parameterSets;
	  int buffer_len = send_buffer.size();
	  parallelLib.bcast_si(buffer_len);
	  parallelLib.bcast_si(send_buffer);
	}
	else { // replace partial list
	  int buffer_len;
	  parallelLib.bcast_si(buffer_len);
	  MPIUnpackBuffer recv_buffer(buffer_len);
	  parallelLib.bcast_si(recv_buffer);
	  recv_buffer >> parameterSets;
	}
      }
    }
  }

  // ---------------------------------------
  // Sequence complete: output final results
  // ---------------------------------------
  if (worldRank == 0) {
    Cout << "\n<<<<< Sequential Hybrid Optimizer Strategy completed.\n";
    // provide a final summary in cases where the default iterator output
    // is insufficient
    if (stratIterDedMaster || numIteratorServers > 1) {// || numIteratorJobs > 1
      size_t i, j, cntr = 0, num_prp_res = prpResults.size(), num_prp_i;
      Cout << "\n<<<<< Sequential hybrid final solution sets:\n";
      for (i=0; i<num_prp_res; ++i) {
	const PRPArray& prp_i = prpResults[i];
	num_prp_i = prp_i.size();
	for (j=0; j<num_prp_i; ++j, ++cntr) {
	  const Variables& vars = prp_i[j].prp_parameters();
	  const Response&  resp = prp_i[j].prp_response();
	  if (!vars.is_null())
	    Cout << "<<<<< Best parameters          (set " << cntr+1
		      << ") =\n" << vars;
	  if (!resp.is_null()) {
	    Cout << "<<<<< Best response functions  (set " << cntr+1
		      << ") =\n";
	    write_data(Cout, resp.function_values());
	  }
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
      Cout << "\n>>>>> Running adaptive Sequential Hybrid Optimizer "
		<< "Strategy with iterator " << methodList[seqCount] << '\n';

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
      Cout << "\n<<<<< Iterator " << methodList[seqCount] << " completed."
		<< "  Progress metric has fallen below threshold.\n";

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
    Cout << "\n<<<<< adaptive Sequential Hybrid Optimizer Strategy "
	      << "completed." << std::endl;
}


void SequentialHybridStrategy::
update_local_results(PRPArray& prp_results, int job_id)
{
  Iterator& curr_iterator = selectedIterators[seqCount];
  Model&    curr_model    = userDefinedModels[seqCount];
  // Analyzers do not currently support returns_multiple_points() since the
  // distinction between Hybrid sampling and Multistart sampling is that
  // the former performs fn evals and processes the data (and current
  // implementations of update_best() only log a single best point).
  if (curr_iterator.returns_multiple_points()) {
    const VariablesArray& vars_results
      = curr_iterator.variables_array_results();
    const ResponseArray& resp_results = curr_iterator.response_array_results();
    // workaround: some methods define vars_results, but not resp_results
    size_t num_vars_results = vars_results.size(),
           num_resp_results = resp_results.size(),
           num_results      = std::max(num_vars_results, num_resp_results);
    prp_results.resize(num_results);
    Variables dummy_vars; Response dummy_resp;
    for (size_t i=0; i<num_results; ++i) {
      const Variables& vars = (num_vars_results) ? vars_results[i] : dummy_vars;
      const Response&  resp = (num_resp_results) ? resp_results[i] : dummy_resp;
      // need a deep copy for case where multiple instances of
      // best{Variables,Response}Array will be assimilated
      prp_results[i] = ParamResponsePair(vars, curr_model.interface_id(),
					 resp, job_id);
    }
  }
  else {
    // need a deep copy for case where multiple instances of
    // best{Variables,Response}Array.front() will be assimilated
    prp_results.resize(1);
    prp_results[0] = ParamResponsePair(curr_iterator.variables_results(),
				       curr_model.interface_id(),
				       curr_iterator.response_results(),job_id);
  }
}

} // namespace Dakota
