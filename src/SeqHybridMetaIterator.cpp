/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SeqHybridMetaIterator
//- Description: Implementation code for the SeqHybridMetaIterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "SeqHybridMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "dakota_data_io.hpp"

static const char rcsId[]="@(#) $Id: SeqHybridMetaIterator.cpp 6972 2010-09-17 22:18:50Z briadam $";


namespace Dakota {

SeqHybridMetaIterator::SeqHybridMetaIterator(ProblemDescDB& problem_db):
  MetaIterator(problem_db)
  //seqHybridType(problem_db.get_string("method.hybrid.type"))
{
  // ***************************************************************************
  // TO DO: support sequences for both Minimizer (solution points) & Analyzer
  // (global SA --> anisotropic UQ): general purpose sequencing with iterator
  // concurrency.
  // ***************************************************************************

  // ***************************************************************************
  // TO DO: once NestedModel has been updated to use IteratorScheduler, consider
  // design using NestedModel lightweight ctor for simple Iterator sequences.
  // Iterators define available I/O and the meta-iterator checks compatibility.
  // ***************************************************************************

  const StringArray& method_ptrs
    = problem_db.get_sa("method.hybrid.method_pointers");
  const StringArray& method_names
    = problem_db.get_sa("method.hybrid.method_names");
  if (!method_ptrs.empty())
    { methodList = method_ptrs;  lightwtCtor = false; }
  else if (!method_names.empty())
    { methodList = method_names; lightwtCtor = true;  }
  size_t i, num_iterators = methodList.size();
  if (!num_iterators) { // verify at least one method in list
    Cerr << "Error: hybrid method list must have a least one entry."<<std::endl;
    abort_handler(-1);
  }

  // define maxIteratorConcurrency without access to selectedIterators
  // (init_iterator_parallelism() requires maxIteratorConcurrency and
  // init_iterator_parallelism() must precede allocate_by_*()).
  size_t sizet_max_replace = 0;
  if (lightwtCtor) {
    // as a first cut, employ the final solutions spec for the seq hybrid
    maxIteratorConcurrency = problem_db.get_sizet("method.final_solutions");
  }
  else {
    maxIteratorConcurrency = 1;
    size_t curr_final, next_concurrency = 1;
    problem_db.set_db_list_nodes(methodList[0]);
    unsigned short curr_method = problem_db.get_ushort("method.algorithm");
    for (i=0; i<num_iterators; ++i) {
      // current method data
      curr_final = problem_db.get_sizet("method.final_solutions");
      // manually replicate iterator-specific defaults, except replace
      // sizet_max with something more useful for concurrency estimation
      if (!curr_final) {
	if (curr_method == MOGA)
	  curr_final = sizet_max_replace
	    = problem_db.get_int("method.population_size");
	else
	  curr_final = 1;
      }

      if (i < num_iterators-1) {
	// look ahead method data
	problem_db.set_db_list_nodes(methodList[i+1]);
	unsigned short next_method = problem_db.get_ushort("method.algorithm");
	// manually replicate accepts_multiple_points() support
	bool next_multi_pt = (next_method == MOGA || next_method == SOGA);
	// allow for multiplicative point growth
	if (next_multi_pt) next_concurrency  = 1;
	else               next_concurrency *= curr_final;
	if (next_concurrency > maxIteratorConcurrency)
	  maxIteratorConcurrency = next_concurrency;
	// updates for next iteration
	curr_method = next_method;
      }
    }
  }
  Cout << "maxIteratorConcurrency = " << maxIteratorConcurrency << '\n';

  iterSched.init_iterator_parallelism(maxIteratorConcurrency);

  if (seqHybridType == "adaptive") {
    if (iterSched.messagePass) {
      // adaptive hybrid does not support iterator concurrency
      Cerr << "Error: adaptive Sequential Hybrid does not support concurrent "
	   << "iterator parallelism." << std::endl;
      abort_handler(-1);
    }
    if (iterSched.iteratorCommRank == 0) {
      progressThreshold
	= problem_db.get_real("method.hybrid.progress_threshold");
      if (progressThreshold > 1.) {
	Cerr << "Warning: progress_threshold should be <= 1.  Setting to 1.\n";
	progressThreshold = 1.;
      }
      else if (progressThreshold < 0.) {
	Cerr << "Warning: progress_threshold should be >= 0.  Setting to 0.\n";
	progressThreshold = 0.;
      }
    }
  }

  // Instantiate all Models and Iterators
  selectedIterators.resize(num_iterators); // slaves also need for run_iterator
  if (lightwtCtor) {
    const String& model_ptr = probDescDB.get_string("method.sub_model_pointer");
    if (!model_ptr.empty())
      problem_db.set_db_model_nodes(model_ptr);
    iteratedModel = problem_db.get_model();
    String empty_model_ptr; // no need to reassign DB model nodes
    for (i=0; i<num_iterators; ++i) // drives need for additional ctor chain
      allocate_by_name(methodList[i], empty_model_ptr,
		       selectedIterators[i], iteratedModel);
  }
  else {
    selectedModels.resize(num_iterators);
    for (i=0; i<num_iterators; ++i)
      allocate_by_pointer(methodList[i], selectedIterators[i],
			  selectedModels[i]);
  }

  // now that parallel paritioning and iterator allocation has occurred,
  // manage acceptable values for Iterator::numFinalSolutions (needed for
  // results_msg_len estimation in run function)
  if (iterSched.iteratorCommRank == 0 && sizet_max_replace) {
    size_t sizet_max = std::numeric_limits<size_t>::max();
    for (i=0; i<num_iterators; ++i)
      if (selectedIterators[i].num_final_solutions() == sizet_max)
	selectedIterators[i].num_final_solutions(sizet_max_replace);
  }
}


SeqHybridMetaIterator::
SeqHybridMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model), lightwtCtor(true) // for now
  //seqHybridType(problem_db.get_string("method.hybrid.type"))
{
  // Hard-wired to lightweight methodList instantiation for now.  To support
  // a more general case indicated by the sequential hybrid spec, will need
  // to validate iteratedModel against any model pointers (--> warnings, see
  // SurrBasedLocalMinimizer for example).

  methodList = problem_db.get_sa("method.hybrid.method_names");
  size_t i, num_iterators = methodList.size();
  if (!num_iterators) { // verify at least one method in list
    Cerr << "Error: hybrid method list must have a least one entry."<<std::endl;
    abort_handler(-1);
  }

  // define maxIteratorConcurrency without access to sub-method specs.
  // as a first cut, employ the final solutions spec for the seq hybrid.
  maxIteratorConcurrency = problem_db.get_sizet("method.final_solutions");
  iterSched.init_iterator_parallelism(maxIteratorConcurrency);

  if (seqHybridType == "adaptive") {
    if (iterSched.messagePass) {
      // adaptive hybrid does not support iterator concurrency
      Cerr << "Error: adaptive Sequential Hybrid does not support concurrent "
	   << "iterator parallelism." << std::endl;
      abort_handler(-1);
    }
    if (iterSched.iteratorCommRank == 0) {
      progressThreshold
	= problem_db.get_real("method.hybrid.progress_threshold");
      if (progressThreshold > 1.) {
	Cerr << "Warning: progress_threshold should be <= 1.  Setting to 1.\n";
	progressThreshold = 1.;
      }
      else if (progressThreshold < 0.) {
	Cerr << "Warning: progress_threshold should be >= 0.  Setting to 0.\n";
	progressThreshold = 0.;
      }
    }
  }

  // Instantiate all Models and Iterators
  selectedIterators.resize(num_iterators); // slaves also need for run_iterator
  String empty_model_ptr; // no need to reassign DB model nodes
  for (i=0; i<num_iterators; ++i) // drives need for additional ctor chain
    allocate_by_name(methodList[i], empty_model_ptr,
		     selectedIterators[i], iteratedModel);
}


SeqHybridMetaIterator::~SeqHybridMetaIterator()
{
  // Virtual destructor handles referenceCount at Iterator level.

  size_t i, num_iterators = methodList.size();
  if (lightwtCtor)
    for (i=0; i<num_iterators; ++i)
      deallocate(selectedIterators[i], iteratedModel);
  else
    for (i=0; i<num_iterators; ++i)
      deallocate(selectedIterators[i], selectedModels[i]);
}


void SeqHybridMetaIterator::core_run()
{
  if (seqHybridType == "adaptive") run_sequential_adaptive();
  else                             run_sequential();
}


/** In the sequential nonadaptive case, there is no interference with
    the iterators.  Each runs until its own convergence criteria is
    satisfied.  Status: fully operational. */
void SeqHybridMetaIterator::run_sequential()
{
  ParallelLibrary& parallel_lib = probDescDB.parallel_library();
  bool lead_rank = iterSched.lead_rank();
  size_t num_iterators = methodList.size();
  for (seqCount=0; seqCount<num_iterators; seqCount++) {

    // each of these is safe for all processors
    Iterator& curr_iterator = selectedIterators[seqCount];
    Model&    curr_model    = (lightwtCtor) ? iteratedModel :
      selectedModels[seqCount];
    bool curr_accepts_multi = curr_iterator.accepts_multiple_points();
    bool curr_returns_multi = curr_iterator.returns_multiple_points();
 
    if (lead_rank)
      Cout << "\n>>>>> Running Sequential Hybrid with iterator "
	   << methodList[seqCount] << ".\n";

    // -------------------------------------------------------------
    // Define total number of runs for this iterator in the sequence
    // -------------------------------------------------------------
    // > run 1st iterator as is, using single default starting pt
    // > subsequent iteration may involve multipoint data flow
    // > In the future, we may support concurrent multipoint iterators, but
    //   prior to additional specification data, we either have a single
    //   multipoint iterator or concurrent single-point iterators.
    if (seqCount == 0) // initialize numIteratorJobs
      iterSched.numIteratorJobs = 1;
    else {
      // update numIteratorJobs
      if (iterSched.iteratorScheduling == MASTER_SCHEDULING) {
	// send curr_accepts_multi from 1st iterator master to strategy master
	if (iterSched.iteratorCommRank == 0 && iterSched.iteratorServerId == 1){
	  int multi_flag = (int)curr_accepts_multi; // bool -> int
	  parallel_lib.send_si(multi_flag, 0, 0);
	}
	else if (iterSched.iteratorServerId == 0) {
	  int multi_flag; MPI_Status status;
	  parallel_lib.recv_si(multi_flag, 1, 0, status);
	  curr_accepts_multi = (bool)multi_flag; // int -> bool
	  iterSched.numIteratorJobs
	    = (curr_accepts_multi) ? 1 : parameterSets.size();
	}
      }
      else { // static scheduling
	if (iterSched.iteratorCommRank == 0)
	  iterSched.numIteratorJobs
	    = (curr_accepts_multi) ? 1 : parameterSets.size();
	// bcast numIteratorJobs over iteratorComm
	if (iterSched.iteratorCommSize > 1)
	  parallel_lib.bcast_i(iterSched.numIteratorJobs);
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
    if (iterSched.iteratorCommRank == 0)
      prpResults.resize(iterSched.numIteratorJobs);

    // -----------------------------------------
    // Define buffer lengths for message passing
    // -----------------------------------------
    if (iterSched.messagePass && iterSched.iteratorCommRank == 0) {
      int params_msg_len, results_msg_len;
      // define params_msg_len
      if (iterSched.iteratorScheduling == MASTER_SCHEDULING) {
	MPIPackBuffer params_buffer;
	pack_parameters_buffer(params_buffer, 0);
	params_msg_len = params_buffer.size();
      }
      // define results_msg_len
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
      results_msg_len = results_buffer.size();
      // publish lengths to IteratorScheduler
      iterSched.iterator_message_lengths(params_msg_len, results_msg_len);
    }

    // ---------------------------------------------------
    // Schedule the runs for this iterator in the sequence
    // ---------------------------------------------------
    iterSched.schedule_iterators(*this, curr_iterator);

    // ---------------------------------
    // Post-process the iterator results
    // ---------------------------------
    // convert prpResults to parameterSets for next iteration
    if (iterSched.iteratorCommRank == 0 && seqCount+1 < num_iterators) {
      size_t i, j, num_param_sets = 0, cntr = 0, num_prp_i;
      for (i=0; i<iterSched.numIteratorJobs; ++i)
	num_param_sets += prpResults[i].size();
      parameterSets.resize(num_param_sets);
      for (i=0; i<iterSched.numIteratorJobs; ++i) {
	const PRPArray& prp_results_i = prpResults[i];
	num_prp_i = prp_results_i.size();
	for (j=0; j<num_prp_i; ++j, ++cntr)
	  parameterSets[cntr] = prp_results_i[j].prp_parameters();
      }
      // migrate results among procs as required for parallel scheduling, e.g.,
      // from multiple single-point iterators to a single multi-point iterator
      // > for dedicated master scheduling, all results data resides on the
      //   dedicated master and no additional migration is required.
      // > for peer static scheduling, the full parameterSets array needs to be
      //   propagated back to peers 2 though n (like an All-Reduce, except that
      //   IteratorScheduler::static_schedule_iterators() enforces reduction to
      //   peer 1 and the code below enforces repropagation from 1 to 2-n).
      if (iterSched.iteratorScheduling == PEER_SCHEDULING &&
	  iterSched.numIteratorServers > 1) {
	if (iterSched.iteratorServerId == 1) { // send complete list
	  MPIPackBuffer send_buffer;
	  send_buffer << parameterSets;
	  int buffer_len = send_buffer.size();
	  parallel_lib.bcast_si(buffer_len);
	  parallel_lib.bcast_si(send_buffer);
	}
	else { // replace partial list
	  int buffer_len;
	  parallel_lib.bcast_si(buffer_len);
	  MPIUnpackBuffer recv_buffer(buffer_len);
	  parallel_lib.bcast_si(recv_buffer);
	  recv_buffer >> parameterSets;
	}
      }
    }
  }

  // ---------------------------------------
  // Sequence complete: output final results
  // ---------------------------------------
  if (lead_rank) {
    Cout << "\n<<<<< Sequential Hybrid completed.\n";
    // provide a final summary in cases where the default iterator output
    // is insufficient
    if (iterSched.messagePass) {// || numIteratorJobs > 1
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
	    Cout << "<<<<< Best response functions  (set " << cntr+1 << ") =\n";
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
void SeqHybridMetaIterator::run_sequential_adaptive()
{
  // NOTE 1: The case where the iterator's internal convergence criteria are 
  // satisfied BEFORE the progress_metric must be either handled or prevented.

  // NOTE 2: Parallel iterator scheduling is not currently supported (and this
  // code will fail if non-default iterator servers or scheduling is specified).

  ParallelLibrary& parallel_lib = probDescDB.parallel_library();
  bool lead_rank = iterSched.lead_rank();
  size_t num_iterators = methodList.size();
  progressMetric = 1.0;
  for (seqCount=0; seqCount<num_iterators; seqCount++) {

    if (lead_rank) {
      Cout << "\n>>>>> Running adaptive Sequential Hybrid with iterator "
	   << methodList[seqCount] << '\n';

      // TO DO: don't run on ded master (see NOTE 2 above)
      //if (iterSched.iteratorServerId) {

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
      if (seqCount+1 < num_iterators) {//prevent index out of range on last pass
        // Get best pt. from completed iteration.
        Variables vars_star = selectedIterators[seqCount].variables_results();
        // Set best pt. as starting point for subsequent iterator
        selectedModels[seqCount+1].active_variables(vars_star);
      }

      // Send the termination message to the servers for this iterator/model
      selectedModels[seqCount].stop_servers();
    }
    else
      iterSched.run_iterator(selectedIterators[seqCount],
	parallel_lib.parallel_configuration().si_parallel_level());
  }

  // Output interesting iterator statistics/progress metrics...
  if (lead_rank)
    Cout << "\n<<<<< adaptive Sequential Hybrid completed." << std::endl;
}


void SeqHybridMetaIterator::
update_local_results(PRPArray& prp_results, int job_id)
{
  Iterator& curr_iterator = selectedIterators[seqCount];
  Model&    curr_model    = selectedModels[seqCount];
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
