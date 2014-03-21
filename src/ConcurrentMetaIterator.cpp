/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
  MetaIterator(problem_db)
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
  const RealVector& param_sets
    = problem_db.get_rv("method.concurrent.parameter_sets");
  int num_random_sets = problem_db.get_int("method.concurrent.random_jobs");

  // Instantiate the model.  A model is used on all processors, even a
  // dedicated master.
  const String& concurr_iter_ptr
    = problem_db.get_string("method.sub_method_pointer");
  const String& concurr_iter_name
    = problem_db.get_string("method.sub_method_name");
  const String& model_ptr = problem_db.get_string("method.sub_model_pointer");
  int param_set_len; size_t restore_index;
  if (!concurr_iter_ptr.empty()) {
    lightwtCtor = false;
    restore_index = problem_db.get_db_method_node(); // for restoration
    problem_db.set_db_list_nodes(concurr_iter_ptr);
  }
  else if (!concurr_iter_name.empty()) {
    lightwtCtor = true;
    if (!model_ptr.empty()) {
      restore_index = problem_db.get_db_model_node(); // for restoration
      problem_db.set_db_model_nodes(model_ptr);
    }
  }
  else {
    if (problem_db.parallel_library().world_rank() == 0)
      Cerr << "Error: insufficient method identification in "
	   << "ConcurrentMetaIterator." << std::endl;
    abort_handler(-1);
  }
  iteratedModel = problem_db.get_model();
  initialize_model(param_set_len);

  // estimation of param_set_len is dependent on the iteratedModel
  // --> concurrent iterator partitioning is pushed downstream a bit
  maxIteratorConcurrency = iterSched.numIteratorJobs
    = param_sets.length() / param_set_len + num_random_sets;
  if (!maxIteratorConcurrency) { // verify at least 1 job has been specified
    if (problem_db.parallel_library().world_rank() == 0)
      Cerr << "Error: concurrent meta-iterator must have at least 1 job.  "
	   << "Please specify either a\n       list of parameter sets or a "
	   << "number of random jobs." << std::endl;
    abort_handler(-1);
  }
  iterSched.init_iterator_parallelism(maxIteratorConcurrency);
  summaryOutputFlag = iterSched.lead_rank();

  // Instantiate the iterator.
  ParallelLibrary& parallel_lib = problem_db.parallel_library();
  if (lightwtCtor) {
    iterSched.init_iterator(concurr_iter_name, selectedIterator, iteratedModel,
      parallel_lib.parallel_configuration().si_parallel_level());
    if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
      Cout << "Concurrent Iterator = " << concurr_iter_name << std::endl;
    if (!model_ptr.empty())
      problem_db.set_db_model_nodes(restore_index); // restore
  }
  else {
    iterSched.init_iterator(problem_db, selectedIterator, iteratedModel,
      parallel_lib.parallel_configuration().si_parallel_level());
    if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
      Cout << "Concurrent Iterator = "
	   << method_enum_to_string(problem_db.get_ushort("method.algorithm"))
	   << std::endl;
    problem_db.set_db_list_nodes(restore_index);    // restore
  }

  initialize(param_set_len);
}


ConcurrentMetaIterator::
ConcurrentMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model), lightwtCtor(true) // for now
{
  // Hard-wired to lightweight methodList instantiation for now.  To support
  // a more general case indicated by the sequential hybrid spec, will need
  // to validate iteratedModel against any model pointers (--> warnings, see
  // SurrBasedLocalMinimizer for example).

  const String& concurr_iter_name
    = problem_db.get_string("method.sub_method_name");
  //const String& model_ptr = problem_db.get_string("method.sub_model_pointer");

  int param_set_len;
  initialize_model(param_set_len);

  maxIteratorConcurrency = iterSched.numIteratorJobs
    = problem_db.get_rv("method.concurrent.parameter_sets").length()
    / param_set_len + problem_db.get_int("method.concurrent.random_jobs");
  if (!maxIteratorConcurrency) { // verify at least 1 job has been specified
    if (problem_db.parallel_library().world_rank() == 0)
      Cerr << "Error: concurrent meta-iterator must have at least 1 job.  "
	   << "Please specify either a\n       list of parameter sets or a "
	   << "number of random jobs." << std::endl;
    abort_handler(-1);
  }
  iterSched.init_iterator_parallelism(maxIteratorConcurrency);
  summaryOutputFlag = iterSched.lead_rank();

  // Instantiate the iterator
  ParallelLibrary& parallel_lib = problem_db.parallel_library();
  iterSched.init_iterator(concurr_iter_name, selectedIterator, iteratedModel,
    parallel_lib.parallel_configuration().si_parallel_level());
  if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
    Cout << "Concurrent Iterator = " << concurr_iter_name << std::endl;

  initialize(param_set_len);
}


void ConcurrentMetaIterator::initialize(int param_set_len)
{
  // initialize initialPt
  if (methodName != MULTI_START)
    copy_data(iteratedModel.continuous_variables(), initialPt); // view->copy

  // estimate params_msg_len and results_msg_len
  if (iterSched.iteratorCommRank == 0) {
    int params_msg_len, results_msg_len;
    // define params_msg_len
    if (iterSched.iteratorScheduling == MASTER_SCHEDULING) {
      RealVector rv(param_set_len);
      MPIPackBuffer send_buffer;
      send_buffer << rv;
      params_msg_len = send_buffer.size();
    }
    // define results_msg_len
    if (iterSched.iteratorScheduling == MASTER_SCHEDULING &&
	iterSched.iteratorServerId == 0) // scheduler proc
      iteratedModel.estimate_message_lengths(); // init_comms not called
    results_msg_len = iteratedModel.message_lengths()[3];
    // publish lengths to IteratorScheduler
    iterSched.iterator_message_lengths(params_msg_len, results_msg_len);
  }

  // -------------------------------------------------------------------------
  // Define parameterSets from the combination of user-specified & random jobs
  // -------------------------------------------------------------------------
  if ( ( iterSched.iteratorScheduling == MASTER_SCHEDULING &&
	 iterSched.iteratorServerId == 0 ) ||
       ( iterSched.iteratorScheduling == PEER_SCHEDULING &&
	 iterSched.iteratorCommRank == 0 ) ) {

    // user-specified jobs
    copy_data(probDescDB.get_rv("method.concurrent.parameter_sets"),
	      parameterSets, 0, param_set_len);

    // random jobs
    int num_random_jobs = probDescDB.get_int("method.concurrent.random_jobs");
    if (num_random_jobs) { // random jobs specified
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
	  lower_bnds.sizeUninitialized(param_set_len); lower_bnds = 0.;
	  upper_bnds.sizeUninitialized(param_set_len); upper_bnds = 1.;
	}
	// invoke NonDLHSSampling as either old or new LHS is always available.
	// We don't use a dace_method_pointer spec since we aren't sampling over
	// the variables specification in all cases.  In particular, we're
	// sampling over multiobj. weight sets in the Pareto-set case.  This
	// hard-wiring currently restricts us to uniform, uncorrelated samples.
        int seed = probDescDB.get_int("method.random_seed");
	unsigned short sample_type = SUBMETHOD_DEFAULT;
	String rng; // empty string: use default
	NonDLHSSampling lhs_sampler(sample_type, num_random_jobs, seed, rng,
				    lower_bnds, upper_bnds);
	const RealMatrix& all_samples = lhs_sampler.all_samples();
	random_jobs.resize(num_random_jobs);
	for (i=0; i<num_random_jobs; ++i)
	  copy_data(all_samples[i], param_set_len, random_jobs[i]);
      }

      if (iterSched.iteratorScheduling == PEER_SCHEDULING &&
	  iterSched.numIteratorServers > 1) {
	// For static scheduling, bcast all random jobs over si_intra_comm (not 
	// necessary for self-scheduling as jobs are assigned from the master).
	if (iterSched.lead_rank()) {
	  MPIPackBuffer send_buffer;
	  send_buffer << random_jobs;
	  int buffer_len = send_buffer.size();
	  iterSched.parallelLib.bcast_si(buffer_len);
	  iterSched.parallelLib.bcast_si(send_buffer);
	}
	else {
	  int buffer_len;
	  iterSched.parallelLib.bcast_si(buffer_len);
	  MPIUnpackBuffer recv_buffer(buffer_len);
	  iterSched.parallelLib.bcast_si(recv_buffer);
	  recv_buffer >> random_jobs;
	}
      }

      // rescale (if needed) and append to parameterSets
      size_t cntr = parameterSets.size();
      parameterSets.resize(iterSched.numIteratorJobs);
      for (i=0; i<num_random_jobs; ++i) {
        if (methodName == MULTI_START)
          parameterSets[cntr] = random_jobs[i];
        else { // scale: multi-objective weights should add to 1
          // NOTE: there is a better way to do this; i.e., mixture experimental
          // design (e.g., Ch. 11 in Myers and Montgomery), but scaling an LHS
          // design is sufficient as a first cut.
          Real sum = 0.0;
          for (j=0; j<param_set_len; j++)
            sum += random_jobs[i][j];
          parameterSets[cntr].sizeUninitialized(param_set_len);
          for (j=0; j<param_set_len; j++)
            parameterSets[cntr][j] = random_jobs[i][j]/sum;
        }
        cntr++;
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
  if (iterSched.iteratorCommRank == 0)
    prpResults.resize(iterSched.numIteratorJobs);
}


ConcurrentMetaIterator::~ConcurrentMetaIterator()
{
  // Virtual destructor handles referenceCount at Iterator level.

  // Free the communicators once for all selectedIterator executions.
  // The strategy dedicated master processor is excluded.
  ParallelLibrary& parallel_lib = probDescDB.parallel_library();
  iterSched.free_iterator(selectedIterator,
    parallel_lib.parallel_configuration().si_parallel_level());

  // si_pl parallelism level is deallocated in ~MetaIterator
}


void ConcurrentMetaIterator::core_run()
{ iterSched.schedule_iterators(*this, selectedIterator); }


void ConcurrentMetaIterator::print_results(std::ostream& s)
{
  using std::setw;
  s << "\n<<<<< Results summary:\n";

  // Table header:
  StringMultiArrayConstView cv_labels
    = prpResults[0].prp_parameters().continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = prpResults[0].prp_parameters().discrete_int_variable_labels();
  StringMultiArrayConstView drv_labels
    = prpResults[0].prp_parameters().discrete_real_variable_labels();
  const StringArray& fn_labels = prpResults[0].prp_response().function_labels();
  size_t i, param_set_len = parameterSets[0].length(),
    num_cv  = cv_labels.size(),  num_div = div_labels.size(),
    num_drv = drv_labels.size(), num_fns = fn_labels.size();
  s << "   set_id "; // matlab comment syntax
  for (i=0; i<param_set_len; ++i) {
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
    for (size_t j=0; j<param_set_len; ++j)
      s << setw(14) << parameterSets[i][j] << ' ';
    const Variables& prp_vars = prp_result.prp_parameters();
    //prp_vars.write_tabular(s) not used since active vars, not all vars
    write_data_tabular(s, prp_vars.continuous_variables());
    write_data_tabular(s, prp_vars.discrete_int_variables());
    write_data_tabular(s, prp_vars.discrete_real_variables());
    prp_result.prp_response().write_tabular(s);
  }
  s << '\n';
}

} // namespace Dakota
