/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ConcurrentMetaIterator
//- Description: Implementation code for the ConcurrentMetaIterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "ConcurrentMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "NonDLHSSampling.hpp"

static const char rcsId[]="@(#) $Id: ConcurrentMetaIterator.cpp 7018 2010-10-12 02:25:22Z mseldre $";


namespace Dakota {

ConcurrentMetaIterator::ConcurrentMetaIterator(ProblemDescDB& problem_db):
  MetaIterator(problem_db),
  numRandomJobs(probDescDB.get_int("method.concurrent.random_jobs")),
  randomSeed(probDescDB.get_int("method.random_seed"))
{
  // ***************************************************************************
  // TO DO: support concurrent meta-iteration for both Minimizer & Analyzer:
  // one step within general purpose module for sequencing with concurrency.
  //   Pareto set:  Minimizer only with obj/lsq weight sets
  //   Multi-start: currently Minimizer; examples of multi-start Analyzer?
  // ***************************************************************************

  // ***************************************************************************
  // TO DO: once NestedModel has been updated to use IteratorScheduler, consider
  // design using NestedModel lightweight ctor for Iterator concurrency.
  // Iterators define available I/O and the meta-iterator checks compatibility.
  // ***************************************************************************

  // pull these from the DB before any resetting of DB nodes
  const RealVector& raw_param_sets
    = problem_db.get_rv("method.concurrent.parameter_sets");

  const String& sub_meth_ptr
    = problem_db.get_string("method.sub_method_pointer");
  const String& sub_meth_name = problem_db.get_string("method.sub_method_name");
  const String& sub_model_ptr
    = problem_db.get_string("method.sub_model_pointer");

  // store/restore the method/model indices separately (the current state of the
  // iterator/model DB nodes may not be synched due to Model ctor recursions in
  // process)
  size_t method_index, model_index; // Note: _NPOS is a valid restoration value
  bool restore_method = false, restore_model = false;
  bool print_rank = (parallelLib.world_rank() == 0); // prior to lead_rank()
  if (!sub_meth_ptr.empty()) {
    restore_method = restore_model = true;
    method_index = problem_db.get_db_method_node(); // for restoration
    model_index  = problem_db.get_db_model_node();  // for restoration
    problem_db.set_db_list_nodes(sub_meth_ptr);
  }
  else if (!sub_meth_name.empty()) {
    // if sub_model_ptr is defined, activate this model spec;
    // if sub_model_ptr is empty, identify default model using empty string.
    // Note: inheriting a prev model spec could be desirable in some contexts,
    //       but not this ctor since a top-level/non-subordinate MetaIterator.
    restore_model = true;
    model_index   = problem_db.get_db_model_node(); // for restoration
    problem_db.set_db_model_nodes(sub_model_ptr);
  }
  else {
    if (print_rank)
      Cerr << "Error: insufficient method identification in "
	   << "ConcurrentMetaIterator." << std::endl;
    abort_handler(-1);
  }

  // Instantiate the model on all processors, even a dedicated master
  iteratedModel = problem_db.get_model();
  initialize_model();

  // user-specified jobs
  copy_data(raw_param_sets, parameterSets, 0, paramSetLen);

  // estimation of paramSetLen is dependent on the iteratedModel
  // --> concurrent iterator partitioning is pushed downstream a bit
  maxIteratorConcurrency = iterSched.numIteratorJobs
    = parameterSets.size() + numRandomJobs;
  if (!maxIteratorConcurrency) { // verify at least 1 job has been specified
    if (print_rank)
      Cerr << "Error: concurrent meta-iterator must have at least 1 job.  "
	   << "Please specify either a\n       list of parameter sets or a "
	   << "number of random jobs." << std::endl;
    abort_handler(-1);
  }

  // restore list nodes
  if (restore_method) problem_db.set_db_method_node(method_index);
  if (restore_model)  problem_db.set_db_model_nodes(model_index);
}


ConcurrentMetaIterator::
ConcurrentMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model),
  numRandomJobs(probDescDB.get_int("method.concurrent.random_jobs")),
  randomSeed(probDescDB.get_int("method.random_seed"))
{
  const RealVector& raw_param_sets
    = problem_db.get_rv("method.concurrent.parameter_sets");

  // ensure consistency between model and any method/model pointers
  check_model(problem_db.get_string("method.sub_method_pointer"),
	      problem_db.get_string("method.sub_model_pointer"));
  // For this ctor with an incoming model, we can simplify DB node assignment
  // and mirror logic in check_model()
  size_t model_index = problem_db.get_db_model_node();  // for restoration
  problem_db.set_db_model_nodes(iteratedModel.model_id());

  initialize_model(); // uses DB lookup for model data (number of obj fns)

  // user-specified jobs
  copy_data(raw_param_sets, parameterSets, 0, paramSetLen);

  maxIteratorConcurrency = iterSched.numIteratorJobs
    = parameterSets.size() + numRandomJobs;
  if (!maxIteratorConcurrency) { // verify at least 1 job has been specified
    if (parallelLib.world_rank() == 0) // prior to lead_rank()
      Cerr << "Error: concurrent meta-iterator must have at least 1 job.  "
	   << "Please specify either a\n       list of parameter sets or a "
	   << "number of random jobs." << std::endl;
    abort_handler(-1);
  }

  // restore list nodes
  problem_db.set_db_model_nodes(model_index);
}


ConcurrentMetaIterator::~ConcurrentMetaIterator()
{ } // Virtual destructor handles referenceCount at Iterator level


void ConcurrentMetaIterator::derived_init_communicators(ParLevLIter pl_iter)
{
  const String& sub_meth_ptr
    = probDescDB.get_string("method.sub_method_pointer");
  const String& sub_meth_name = probDescDB.get_string("method.sub_method_name");
  //const String& sub_model_ptr
  //  = probDescDB.get_string("method.sub_model_pointer");

  // Model recursions may update method or model nodes and restoration may not
  // occur until the recursion completes, so don't assume that method and
  // model indices are in sync.
  size_t method_index, model_index; // Note: _NPOS is a valid restoration value
  bool restore_method = false, restore_model = false,
    lightwt_ctor = sub_meth_ptr.empty();
  if (lightwt_ctor) {
    restore_model = true;
    model_index = probDescDB.get_db_model_node(); // for restoration
    probDescDB.set_db_model_nodes(iteratedModel.model_id());
  }
  else {
    restore_method = restore_model = true;
    method_index = probDescDB.get_db_method_node(); // for restoration
    model_index  = probDescDB.get_db_model_node();  // for restoration
    probDescDB.set_db_list_nodes(sub_meth_ptr);
  }

  iterSched.update(methodPCIter);

  // It is not practical to estimate the evaluation concurrency without 
  // instantiating the iterator (see, e.g., NonDPolynomialChaos), and here we
  // have a circular dependency: we need the evaluation concurrency from the
  // iterator to estimate the max_ppi for input to the mi_pl partition, but
  // we want to segregate iterator construction based on the mi_pl partition.
  // To resolve this dependency, we instantiate the iterator based on the
  // previous parallel level and then augment it below based on the mi_pl level.
  // We avoid repeated instantiations by the check on iterator.is_null() as well
  // as through the lookup in problem_db_get_iterator() (method_ptr case); this
  // requires that no calls to init_comms occur at construct time, since the
  // mi_pl basis for this is not yet available.
  IntIntPair ppi_pr = (lightwt_ctor) ?
    iterSched.configure(probDescDB, sub_meth_name, selectedIterator,
			iteratedModel) :
    iterSched.configure(probDescDB, selectedIterator, iteratedModel);
  iterSched.partition(maxIteratorConcurrency, ppi_pr);
  summaryOutputFlag = iterSched.lead_rank();

  // from this point on, we can specialize logic in terms of iterator servers.
  // An idle partition need not instantiate iterators (empty selectedIterator
  // envelope is adequate) or initialize, so return now.  A dedicated
  // master processor is managed in IteratorScheduler::init_iterator().
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    // Instantiate the iterator
    if (lightwt_ctor) {
      iterSched.init_iterator(probDescDB, sub_meth_name, selectedIterator,
			      iteratedModel);
      if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
	Cout << "Concurrent Iterator = " << sub_meth_name << std::endl;
    }
    else {
      iterSched.init_iterator(probDescDB, selectedIterator, iteratedModel);
      if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
	Cout << "Concurrent Iterator = "
	     << method_enum_to_string(probDescDB.get_ushort("method.algorithm"))
	     << std::endl;
    }
  }

  // restore list nodes
  if (restore_method) probDescDB.set_db_method_node(method_index);
  if (restore_model)  probDescDB.set_db_model_nodes(model_index);
}


void ConcurrentMetaIterator::derived_set_communicators(ParLevLIter pl_iter)
{
  size_t mi_pl_index = methodPCIter->mi_parallel_level_index(pl_iter) + 1;
  iterSched.update(methodPCIter, mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    iterSched.set_iterator(selectedIterator, si_pl_iter);
  }
}


void ConcurrentMetaIterator::derived_free_communicators(ParLevLIter pl_iter)
{
  size_t mi_pl_index = methodPCIter->mi_parallel_level_index(pl_iter) + 1;
  iterSched.update(methodPCIter, mi_pl_index);
  if (iterSched.iteratorServerId <= iterSched.numIteratorServers) {
    ParLevLIter si_pl_iter
      = methodPCIter->mi_parallel_level_iterator(mi_pl_index);
    iterSched.free_iterator(selectedIterator, si_pl_iter);
  }

  // deallocate the mi_pl parallelism level
  iterSched.free_iterator_parallelism();
}


void ConcurrentMetaIterator::pre_run()
{
  if (iterSched.iteratorCommRank > 0 ||
      iterSched.iteratorServerId > iterSched.numIteratorServers)
    return;

  // initialize initialPt
  if (methodName != MULTI_START)
    copy_data(iteratedModel.continuous_variables(), initialPt); // view->copy

  // estimate params_msg_len & results_msg_len and publish to IteratorScheduler
  int params_msg_len = 0, results_msg_len; // peer sched doesn't send params
  if (iterSched.iteratorScheduling == MASTER_SCHEDULING) {
    // define params_msg_len
    RealVector rv(paramSetLen);
    MPIPackBuffer send_buffer;
    send_buffer << rv;
    params_msg_len = send_buffer.size();
    // define results_msg_len
    if (iterSched.iteratorServerId == 0) // master proc: init_comms not called
      iteratedModel.estimate_message_lengths();
  }
  results_msg_len = iteratedModel.message_lengths()[3];
  iterSched.iterator_message_lengths(params_msg_len, results_msg_len);

  // -------------------------------------------------------------------------
  // Define parameterSets from the combination of user-specified & random jobs
  // -------------------------------------------------------------------------
  if ( iterSched.iteratorServerId   == 0 ||                // master proc
       iterSched.iteratorScheduling == PEER_SCHEDULING ) { // peer server

    // random jobs
    if (numRandomJobs) { // random jobs specified
      size_t i, j;
      RealVectorArray random_jobs;
      if (iterSched.lead_rank()) {
	// set up bounds for uniform sampling
	RealVector lower_bnds, upper_bnds;
	if (methodName == MULTI_START) {
	  lower_bnds = iteratedModel.continuous_lower_bounds(); // view OK
	  upper_bnds = iteratedModel.continuous_upper_bounds(); // view OK
	}
	else {
	  lower_bnds.sizeUninitialized(paramSetLen); lower_bnds = 0.;
	  upper_bnds.sizeUninitialized(paramSetLen); upper_bnds = 1.;
	}
	// invoke NonDLHSSampling as either old or new LHS is always available.
	// We don't use a dace_method_pointer spec since we aren't sampling over
	// the variables specification in all cases.  In particular, we're
	// sampling over multiobj. weight sets in the Pareto-set case.  This
	// hard-wiring currently restricts us to uniform, uncorrelated samples.
	unsigned short sample_type = SUBMETHOD_DEFAULT;
	String rng; // empty string: use default
	NonDLHSSampling lhs_sampler(sample_type, numRandomJobs, randomSeed,
				    rng, lower_bnds, upper_bnds);
	const RealMatrix& all_samples = lhs_sampler.all_samples();
	random_jobs.resize(numRandomJobs);
	for (i=0; i<numRandomJobs; ++i)
	  copy_data(all_samples[i], paramSetLen, random_jobs[i]);
      }

      if (iterSched.iteratorScheduling == PEER_SCHEDULING &&
	  iterSched.numIteratorServers > 1) {
	const ParallelLevel& mi_pl
	  = methodPCIter->mi_parallel_level(iterSched.miPLIndex);
	// For static scheduling, bcast all random jobs over mi_intra_comm (not 
	// necessary for self-scheduling as jobs are assigned from the master).
	if (iterSched.lead_rank()) {
	  MPIPackBuffer send_buffer;
	  send_buffer << random_jobs;
	  int buffer_len = send_buffer.size();
	  parallelLib.bcast_hs(buffer_len, mi_pl);
	  parallelLib.bcast_hs(send_buffer, mi_pl);
	}
	else {
	  int buffer_len;
	  parallelLib.bcast_hs(buffer_len, mi_pl);
	  MPIUnpackBuffer recv_buffer(buffer_len);
	  parallelLib.bcast_hs(recv_buffer, mi_pl);
	  recv_buffer >> random_jobs;
	}
      }

      // rescale (if needed) and append to parameterSets
      size_t cntr = parameterSets.size();
      parameterSets.resize(iterSched.numIteratorJobs);
      for (i=0; i<numRandomJobs; ++i, ++cntr) {
        if (methodName == MULTI_START)
          parameterSets[cntr] = random_jobs[i];
        else { // scale: multi-objective weights should add to 1
          // NOTE: there is a better way to do this; i.e., mixture experimental
          // design (e.g., Ch. 11 in Myers and Montgomery), but scaling an LHS
          // design is sufficient as a first cut.
          Real sum = 0.0;
          for (j=0; j<paramSetLen; j++)
            sum += random_jobs[i][j];
          parameterSets[cntr].sizeUninitialized(paramSetLen);
          for (j=0; j<paramSetLen; j++)
            parameterSets[cntr][j] = random_jobs[i][j]/sum;
        }
      }
    }

    // May also want to support an approach, at least for multistart, where the
    // best m subset out of n evaluated candidates are used for iterator starts.
    // However, this requires evaluation of the n candidates, which violates the
    // distinction between a multi-start minimizer and a sequential hybrid
    // minimizer (initiated with global sampling).
  }

  // all iterator masters bookkeep on the full results list, even if
  // only some entries are defined locally
  prpResults.resize(iterSched.numIteratorJobs);
}


void ConcurrentMetaIterator::core_run()
{
  // For graphics data, limit to iterator server comm leaders; this is further
  // segregated within initialize_graphics(): all iterator masters stream
  // tabular data, but only iterator server 1 generates a graphics window.
  if (iterSched.iteratorCommRank == 0) {
    int server_id = iterSched.iteratorServerId;
    if (server_id > 0 && server_id <= iterSched.numIteratorServers)
      selectedIterator.initialize_graphics(server_id);
  }

  iterSched.schedule_iterators(*this, selectedIterator);
}


void ConcurrentMetaIterator::print_results(std::ostream& s)
{
  using std::setw;
  s << "\n<<<<< Results summary:\n";

  // Table header:
  StringMultiArrayConstView cv_labels
    = prpResults[0].variables().continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = prpResults[0].variables().discrete_int_variable_labels();
  StringMultiArrayConstView drv_labels
    = prpResults[0].variables().discrete_real_variable_labels();
  const StringArray& fn_labels = prpResults[0].response().function_labels();
  size_t i, num_cv  = cv_labels.size(),  num_div = div_labels.size(),
    num_drv = drv_labels.size(), num_fns = fn_labels.size();
  s << "   set_id "; // matlab comment syntax
  for (i=0; i<paramSetLen; ++i) {
    if (methodName == MULTI_START)
      s << setw(14) << cv_labels[i].data() << ' ';
    else {
      char string[10];
      std::sprintf(string, "w%i", (int)i + 1);
      s << setw(14) << string << ' ';
    }
  }
  for (i=0; i<num_cv; i++) {
    String label = (methodName == MULTI_START) ?
      cv_labels[i] + String("*") : cv_labels[i];
    s << setw(14) << label.data() << ' ';
  }
  for (i=0; i<num_div; i++) {
    String label = (methodName == MULTI_START) ?
      div_labels[i] + String("*") : div_labels[i];
    s << setw(14) << label.data() << ' ';
  }
  for (i=0; i<num_drv; i++) {
    String label = (methodName == MULTI_START) ?
      drv_labels[i] + String("*") : drv_labels[i];
    s << setw(14) << label.data() << ' ';
  }
  for (i=0; i<num_fns; i++)
    s << setw(14) << fn_labels[i].data() << ' ';
  s << '\n';

  // Table data:
  size_t num_results = prpResults.size();
  for (i=0; i<num_results; ++i) {
    const ParamResponsePair& prp_result = prpResults[i];
    s << std::setprecision(10) << std::resetiosflags(std::ios::floatfield)
         << setw(9) << prp_result.eval_id() << ' ';
    for (size_t j=0; j<paramSetLen; ++j)
      s << setw(14) << parameterSets[i][j] << ' ';
    const Variables& prp_vars = prp_result.variables();
    //prp_vars.write_tabular(s) not used since active vars, not all vars
    write_data_tabular(s, prp_vars.continuous_variables());
    write_data_tabular(s, prp_vars.discrete_int_variables());
    write_data_tabular(s, prp_vars.discrete_real_variables());
    prp_result.response().write_tabular(s);
  }
  s << '\n';
}

} // namespace Dakota
