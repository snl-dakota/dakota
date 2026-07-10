/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ConcurrentMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "NonDLHSSampling.hpp"
#include "EvaluationStore.hpp"
#include "StudyRuntime.hpp"
#include "IRStore.hpp"
#include "LibraryRuntimeSupport.hpp"
#include <stdexcept>

static const char rcsId[]="@(#) $Id: ConcurrentMetaIterator.cpp 7018 2010-10-12 02:25:22Z mseldre $";


namespace Dakota {

ConcurrentMetaIterator::ConcurrentMetaIterator(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib):
  MetaIterator(problem_db, parallel_lib),
  numRandomJobs(probDescDB.get<int>("method.concurrent.random_jobs")),
  randomSeed(probDescDB.get<int>("method.random_seed"))
{
  // ***************************************************************************
  // TO DO: support concurrent meta-iteration for both Minimizer & Analyzer:
  // one step within general purpose module for sequencing with concurrency.
  //   Pareto set:  Minimizer only with obj/lsq weight sets
  //   Multi-start: currently Minimizer; examples of multi-start Analyzer?
  // ***************************************************************************

  // ***************************************************************************
  // TO DO: once NestedModel has been updated to use IteratorExecutor, consider
  // design using NestedModel lightweight ctor for Iterator concurrency.
  // Iterators define available I/O and the meta-iterator checks compatibility.
  // ***************************************************************************

  // pull these from the DB before any resetting of DB nodes
  const RealVector& raw_param_sets
    = problem_db.get<const RealVector>("method.concurrent.parameter_sets");

  const String& sub_meth_ptr
    = problem_db.get<const String>("method.sub_method_pointer");
  const String& sub_meth_name = problem_db.get<const String>("method.sub_method_name");
  const String& sub_model_ptr
    = problem_db.get<const String>("method.sub_model_pointer");

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

  // Instantiate the model on all processors, even a dedicated scheduler
  iteratedModel = Model::get_model(problem_db, parallel_lib);
  initialize_model();

  // user-specified jobs
  copy_data(raw_param_sets, parameterSets, 0, paramSetLen);

  // estimation of paramSetLen is dependent on the iteratedModel
  // --> concurrent iterator partitioning is pushed downstream a bit
  maxIteratorConcurrency = iterSched.numIteratorJobs()
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
ConcurrentMetaIterator(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model):
  MetaIterator(problem_db, parallel_lib, model),
  numRandomJobs(probDescDB.get<int>("method.concurrent.random_jobs")),
  randomSeed(probDescDB.get<int>("method.random_seed"))
{
  const RealVector& raw_param_sets
    = problem_db.get<const RealVector>("method.concurrent.parameter_sets");

  // ensure consistency between model and any method/model pointers
  check_model(problem_db.get<const String>("method.sub_method_pointer"),
	      problem_db.get<const String>("method.sub_model_pointer"));
  // For this ctor with an incoming model, we can simplify DB node assignment
  // and mirror logic in check_model()
  size_t model_index = problem_db.get_db_model_node();  // for restoration
  problem_db.set_db_model_nodes(iteratedModel->model_id());

  initialize_model(); // uses DB lookup for model data (number of obj fns)

  // user-specified jobs
  copy_data(raw_param_sets, parameterSets, 0, paramSetLen);

  maxIteratorConcurrency = iterSched.numIteratorJobs()
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


ConcurrentMetaIterator::
ConcurrentMetaIterator(const IRStore& method_store,
                       std::shared_ptr<Iterator> sub_iterator,
                       std::shared_ptr<ParallelLibrary> parallel_lib,
                       std::shared_ptr<OutputManager> output_mgr):
  MetaIterator(detail::resolve_runtime(
                 std::move(parallel_lib), std::move(output_mgr),
                 {detail::RuntimeDependency("Iterator",
                                            sub_iterator ? sub_iterator->parallel_library_ptr() : nullptr,
                                            sub_iterator ? sub_iterator->output_manager_ptr() : nullptr),
                  detail::RuntimeDependency("Model",
                                            sub_iterator && sub_iterator->iterated_model() ?
                                              sub_iterator->iterated_model()->parallel_library_ptr() : nullptr,
                                            sub_iterator && sub_iterator->iterated_model() ?
                                              sub_iterator->iterated_model()->output_manager_ptr() : nullptr)},
                 "ConcurrentMetaIterator"),
               method_store,
               sub_iterator ? sub_iterator->iterated_model() : nullptr),
  numRandomJobs(method_store.get<int>("concurrent.random_jobs")),
  randomSeed(method_store.get<int>("random_seed"))
{
  if (!sub_iterator)
    throw std::runtime_error(
      "ConcurrentMetaIterator requires a non-null sub_iterator in DI construction.");

  if (!iteratedModel)
    throw std::runtime_error(
      "ConcurrentMetaIterator requires sub_iterator->iterated_model() in DI construction.");

  selectedIterator = std::move(sub_iterator);

  const RealVector& raw_param_sets =
    method_store.get<RealVector>("concurrent.parameter_sets");

  initialize_model();
  copy_data(raw_param_sets, parameterSets, 0, paramSetLen);

  maxIteratorConcurrency = iterSched.numIteratorJobs()
    = parameterSets.size() + numRandomJobs;
  if (!maxIteratorConcurrency)
    throw std::runtime_error(
      "ConcurrentMetaIterator requires at least one parameter set or random job.");
}


void ConcurrentMetaIterator::declare_sources() {
  evaluationsDB.declare_source(method_id(), 
                               "iterator",
                               selectedIterator->method_id(),
                               "iterator");
}

ConcurrentMetaIterator::~ConcurrentMetaIterator()
{ }


void ConcurrentMetaIterator::derived_init_communicators(ParLevLIter pl_iter)
{
  bool db_backed = (&probDescDB != &dummy_db);
  String sub_meth_ptr, sub_meth_name;
  size_t method_index = 0, model_index = 0;
  bool restore_method = false, restore_model = false;
  bool lightwt_ctor = false;

  if (db_backed) {
    sub_meth_ptr = probDescDB.get<const String>("method.sub_method_pointer");
    sub_meth_name = probDescDB.get<const String>("method.sub_method_name");
    lightwt_ctor = sub_meth_ptr.empty();
    if (lightwt_ctor) {
      restore_model = true;
      model_index = probDescDB.get_db_model_node();
      probDescDB.set_db_model_nodes(iteratedModel->model_id());
    }
    else {
      restore_method = restore_model = true;
      method_index = probDescDB.get_db_method_node();
      model_index  = probDescDB.get_db_model_node();
      probDescDB.set_db_list_nodes(sub_meth_ptr);
    }
  }

  if (db_backed) {
    iterSched.update(methodPCIter);
    IntIntPair ppi_pr = (lightwt_ctor) ?
      iterSched.configure(probDescDB, sub_meth_name, selectedIterator,
                          iteratedModel) :
      iterSched.configure(probDescDB, selectedIterator, iteratedModel);
    iterSched.partition(maxIteratorConcurrency, ppi_pr);
    summaryOutputFlag = iterSched.lead_rank();

    if (iterSched.active_server()) {
      if (lightwt_ctor) {
        iterSched.initialize_iterator(sub_meth_name, selectedIterator,
                                      iteratedModel);
        if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
          Cout << "Concurrent Iterator = " << sub_meth_name << std::endl;
      }
      else {
        iterSched.initialize_iterator(probDescDB, selectedIterator, iteratedModel);
        if (summaryOutputFlag && outputLevel >= VERBOSE_OUTPUT)
          Cout << "Concurrent Iterator = "
               << method_enum_to_string(probDescDB.get<unsigned short>("method.algorithm"))
               << std::endl;
      }
    }
  }
  else {
    iterSched.prepare_child_iterator(
      selectedIterator, methodPCIter, maxIteratorConcurrency);
    summaryOutputFlag = iterSched.lead_rank();
    if (iterSched.active_server() && summaryOutputFlag &&
        outputLevel >= VERBOSE_OUTPUT)
      Cout << "Concurrent Iterator = "
           << selectedIterator->method_string() << std::endl;
  }

  if (restore_method) probDescDB.set_db_method_node(method_index);
  if (restore_model)  probDescDB.set_db_model_nodes(model_index);
}



void ConcurrentMetaIterator::derived_set_communicators(ParLevLIter pl_iter)
{
  iterSched.set_child_iterator(*selectedIterator, methodPCIter, pl_iter);
}


void ConcurrentMetaIterator::derived_free_communicators(ParLevLIter pl_iter)
{
  iterSched.free_child_iterator(*selectedIterator, methodPCIter, pl_iter);
}


IntIntPair ConcurrentMetaIterator::estimate_partition_bounds()
{
  // Note: ConcurrentMetaIterator::derived_init_communicators() calls
  // IteratorExecutor::configure() to estimate_partition_bounds() on the
  // subIterator, not the MetaIterator.  When ConcurrentMetaIterator is a
  // sub-iterator, we augment the subIterator concurrency with the MetaIterator
  // concurrency.  [Thus, this is not redundant with configure().]

  // This function is already rank protected as far as partitioning has occurred
  // to this point.  However, this call may precede derived_init_communicators
  // when the ConcurrentMetaIterator is a sub-iterator.
  if (&probDescDB != &dummy_db)
    iterSched.construct_sub_iterator(probDescDB, parallelLib, selectedIterator, iteratedModel,
      probDescDB.get<const String>("method.sub_method_pointer"),
      probDescDB.get<const String>("method.sub_method_name"),
      probDescDB.get<const String>("method.sub_model_pointer"));
  IntIntPair min_max, si_min_max = selectedIterator->estimate_partition_bounds();

  // now apply scheduling data for this level (recursion is complete)
  min_max.first = ProblemDescDB::min_procs_per_level(si_min_max.first,
    iterSched.procsPerIterator(), iterSched.numIteratorServers());
  min_max.second = ProblemDescDB::max_procs_per_level(si_min_max.second,
    iterSched.procsPerIterator(), iterSched.numIteratorServers(),
    iterSched.iteratorScheduling(), 1, false, maxIteratorConcurrency);
  return min_max;
}


void ConcurrentMetaIterator::initialize_model()
{
  if (methodName == PARETO_SET) {
    paramSetLen = iteratedModel->num_primary_fns();
    // define dummy weights to trigger model recasting in iterator construction
    // (replaced at run-time with weight sets from specification)
    if (iteratedModel->primary_response_fn_weights().empty()) {
      RealVector initial_wts(paramSetLen, false);
      initial_wts = 1./(Real)paramSetLen;
      iteratedModel->primary_response_fn_weights(initial_wts); // trigger recast
    }
  }
  else
    paramSetLen = ModelUtils::cv(*iteratedModel);
}


void ConcurrentMetaIterator::pre_run()
{
  if (!iterSched.iterator_comm_lead() || iterSched.idle_partition())
    return;

  // initialize initialPt
  if (methodName != MULTI_START)
    copy_data(ModelUtils::continuous_variables(*iteratedModel), initialPt); // view->copy

  // estimate params_msg_len & results_msg_len and publish to IteratorExecutor
  int params_msg_len = 0, results_msg_len; // peer sched doesn't send params
  if (iterSched.dedicated_scheduler()) {
    // define params_msg_len
    RealVector rv(paramSetLen);
    MPIPackBuffer send_buffer;
    send_buffer << rv;
    params_msg_len = send_buffer.size();
    // define results_msg_len
    if (iterSched.scheduler_rank())// scheduler proc: init_comms not called
      iteratedModel->estimate_message_lengths();
  }
  results_msg_len = iteratedModel->message_lengths()[3];
  iterSched.iterator_message_lengths(params_msg_len, results_msg_len);

  // -------------------------------------------------------------------------
  // Define parameterSets from the combination of user-specified & random jobs
  // -------------------------------------------------------------------------
  if ( iterSched.scheduler_rank() ||                // ded scheduler
       iterSched.peer_scheduling() ) { // peer server

    // random jobs
    if (numRandomJobs) { // random jobs specified
      size_t i, j;
      RealVectorArray random_jobs;
      if (iterSched.lead_rank()) {
	// set up bounds for uniform sampling
	RealVector lower_bnds, upper_bnds;
	if (methodName == MULTI_START) {
	  lower_bnds = ModelUtils::continuous_lower_bounds(*iteratedModel); // view OK
	  upper_bnds = ModelUtils::continuous_upper_bounds(*iteratedModel); // view OK
	  if (iterSched.lead_rank()) {
	    Cout << "[ConcurrentMetaIterator] random-start bounds lower="
	         << lower_bnds << " upper=" << upper_bnds << std::endl;
	  }
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

      if (iterSched.peer_scheduling() &&
	  iterSched.multiple_iterator_servers()) {
	const ParallelLevel& mi_pl
	  = methodPCIter->mi_parallel_level(iterSched.miPLIndex());
	// For static scheduling, bcast all random jobs over mi_intra_comm (not 
	// necessary for dedicated-scheduler as jobs are assigned from the
	// scheduler).
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
      parameterSets.resize(iterSched.numIteratorJobs());
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

  // all iterator schedulers bookkeep on the full results list, even if
  // only some entries are defined locally
  prpResults.resize(iterSched.numIteratorJobs());
}


void ConcurrentMetaIterator::core_run()
{
  // For graphics data, limit to iterator server comm leaders; this is further
  // segregated within initialize_graphics(): all iterator schedulers stream
  // tabular data, but only iterator server 1 generates a graphics window.
  if (iterSched.graphics_server())
    selectedIterator->initialize_graphics(iterSched.iteratorServerId());

  iterSched.schedule_iterators(*this, *selectedIterator);
}


void ConcurrentMetaIterator::print_results(std::ostream& s, short results_state)
{
  using std::setw;
  s << "\n<<<<< Results summary:\n";

  // Table header:
  StringMultiArrayConstView cv_labels
    = prpResults[0].variables().continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = prpResults[0].variables().discrete_int_variable_labels();
  StringMultiArrayConstView dsv_labels
    = prpResults[0].variables().discrete_string_variable_labels();  
  StringMultiArrayConstView drv_labels
    = prpResults[0].variables().discrete_real_variable_labels();
  const StringArray& fn_labels = prpResults[0].response().function_labels();  
  size_t i, num_cv  = cv_labels.size(),  num_div = div_labels.size(),
    num_dsv = dsv_labels.size(), num_drv = drv_labels.size(), 
    num_fns = fn_labels.size();
  s << "   set_id "; // matlab comment syntax
  StringArray weight_strings; // used to store pareto_set results
  for (i=0; i<paramSetLen; ++i) {
    if (methodName == MULTI_START)
      s << setw(14) << cv_labels[i].data() << ' ';
    else {
      char string[10];
      std::sprintf(string, "w%i", (int)i + 1);
      weight_strings.push_back(string);
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
  for (i=0; i<num_dsv; i++) {
    String label = (methodName == MULTI_START) ?
      dsv_labels[i] + String("*") : dsv_labels[i];
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
    write_data_tabular(s, prp_vars.discrete_string_variables());
    write_data_tabular(s, prp_vars.discrete_real_variables());
    prp_result.response().write_tabular(s);
  }
  s << '\n';
  // Write results to DB
  if(resultsDB.active()) {
    const auto iterator_id = run_identifier();
    // set_ids for dimnension scales
    IntArray set_ids(num_results);
    std::iota(set_ids.begin(), set_ids.end(), 1);
    // Store parameterSets, which are initial points for multi_start and weights
    // for pareto_set.
    DimScaleMap pset_scales;
    StringArray pset_location;
    pset_scales.emplace(0, IntegerScale("set_ids", set_ids));
    if(methodName == MULTI_START) {
      pset_scales.emplace(1, StringScale("variables", cv_labels));
      pset_location = {String("starting_points"), String("continuous")};
    } else {
      pset_scales.emplace(1, StringScale("weights", weight_strings));
      pset_location.push_back("weights");
    }
    resultsDB.allocate_matrix(iterator_id, pset_location, 
        Dakota::ResultsOutputType::REAL, num_results, paramSetLen, pset_scales);
    for(int i = 0; i < num_results; ++i)
      resultsDB.insert_into(iterator_id, pset_location, parameterSets[i],i);
    // Store best continuous variables 
    if(num_cv) {  // should always be true
      DimScaleMap best_parameters_scales;
      best_parameters_scales.emplace(0, IntegerScale("set_id", set_ids));
      best_parameters_scales.emplace(1, StringScale("variables", cv_labels));
      StringArray location = {String("best_parameters"), String("continuous")};
      resultsDB.allocate_matrix(iterator_id, location, Dakota::ResultsOutputType::REAL, 
          num_results, num_cv, best_parameters_scales);
      for(int i = 0; i < num_results; ++i)
        resultsDB.insert_into(iterator_id, location,
            prpResults[i].variables().continuous_variables(), i);
    }
    // Store best discrete ints
    if(num_div) {
      DimScaleMap best_parameters_scales;
      best_parameters_scales.emplace(0, IntegerScale("set_ids", set_ids));
      best_parameters_scales.emplace(1, StringScale("variables", div_labels));
      StringArray location = {String("best_parameters"), String("discrete_integer")};
      resultsDB.allocate_matrix(iterator_id, location, Dakota::ResultsOutputType::INTEGER, 
          num_results, num_div, best_parameters_scales);
      for(int i = 0; i < num_results; ++i)
        resultsDB.insert_into(iterator_id, location,
            prpResults[i].variables().discrete_int_variables(), i);
    }
    // Store best discrete strings
    if(num_dsv) {
      DimScaleMap best_parameters_scales;
      best_parameters_scales.emplace(0, IntegerScale("set_ids", set_ids));
      best_parameters_scales.emplace(1, StringScale("variables", dsv_labels));
      StringArray location = {String("best_parameters"), String("discrete_string")};
      resultsDB.allocate_matrix(iterator_id, location, Dakota::ResultsOutputType::STRING, 
          num_results, num_dsv, best_parameters_scales);
      for(int i = 0; i < num_results; ++i)
        resultsDB.insert_into(iterator_id, location,
            prpResults[i].variables().discrete_string_variables(), i);
    }
    // Store best discrete reals
    if(num_drv) {
      DimScaleMap best_parameters_scales;
      best_parameters_scales.emplace(0, IntegerScale("set_ids", set_ids));
      best_parameters_scales.emplace(1, StringScale("variables", drv_labels));
      StringArray location = {String("best_parameters"), String("discrete_real")};
      resultsDB.allocate_matrix(iterator_id, location, Dakota::ResultsOutputType::REAL, 
          num_results, num_drv, best_parameters_scales);
      for(int i = 0; i < num_results; ++i)
        resultsDB.insert_into(iterator_id, location,
            prpResults[i].variables().discrete_real_variables(), i);
    }
    // Store best responses
    if(num_fns) {
      DimScaleMap best_responses_scales;
      best_responses_scales.emplace(0, IntegerScale("set_ids", set_ids));
      best_responses_scales.emplace(1, StringScale("responses", fn_labels));
      StringArray location = {String("best_responses")};
      resultsDB.allocate_matrix(iterator_id, location, Dakota::ResultsOutputType::REAL, 
          num_results, num_fns, best_responses_scales);
      for(int i = 0; i < num_results; ++i)
        resultsDB.insert_into(iterator_id, location,
            prpResults[i].response().function_values(), i);
    }
  }
}

} // namespace Dakota
