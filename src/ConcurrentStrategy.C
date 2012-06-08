/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ConcurrentStrategy
//- Description: Implementation code for the ConcurrentStrategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "ConcurrentStrategy.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "ParamResponsePair.H"
#include "NonDLHSSampling.H"

static const char rcsId[]="@(#) $Id: ConcurrentStrategy.C 7018 2010-10-12 02:25:22Z mseldre $";


namespace Dakota {

ConcurrentStrategy::ConcurrentStrategy(ProblemDescDB& problem_db):
  Strategy(BaseConstructor(), problem_db)
{
  if (worldRank==0)
    Cout << "Constructing Concurrent Iterator Strategy...\n";

  multiStartFlag = (strategyName == "multi_start");

  const RealVector& raw_param_sets
    = problem_db.get_rv("strategy.concurrent.parameter_sets");
  int num_random_jobs = problem_db.get_int("strategy.concurrent.random_jobs");

  const String& concurrent_iterator
    = problem_db.get_string("strategy.method_pointer");
  if (worldRank==0)
    Cout << "concurrent_iterator = " << concurrent_iterator << std::endl;
  problem_db.set_db_list_nodes(concurrent_iterator);

  int param_set_len;
  if (multiStartFlag) {
    Variables dummy_vars(problem_db);
    param_set_len = dummy_vars.cv();
  }
  else
    param_set_len = problem_db.get_sizet("responses.num_objective_functions");

  maxConcurrency = numIteratorJobs
    = raw_param_sets.length()/param_set_len + num_random_jobs;
  if (!numIteratorJobs) { // verify at least one total job has been specified
    Cerr << "Error: concurrent strategy must have at least 1 job.  Please "
	 << "specify either a\n       list of parameter sets or a number of "
	 << "random jobs." << std::endl;
    abort_handler(-1);
  }
  init_iterator_parallelism();

  // Instantiate the model and iterator.  A model is used on all processors,
  // even a strategy dedicated master.
  userDefinedModel = problem_db.get_model();
  if (strategyName == "pareto_set" && 
      userDefinedModel.primary_response_fn_weights().empty()) {
    // set dummy weights to use in iterator construction (to be
    // replaced at run-time with weight sets from strategy)
    RealVector initial_wts(param_set_len, false);
    initial_wts = 1./(Real)param_set_len;
    userDefinedModel.primary_response_fn_weights(initial_wts);
  }
  // Initialize once for all selectedIterator executions.
  // The strategy dedicated master processor is excluded.
  if (!stratIterDedMaster || worldRank)
    init_iterator(selectedIterator, userDefinedModel);

  // initialize initialPt, paramsMsgLen, and resultsMsgLen
  if (!multiStartFlag)
    copy_data(userDefinedModel.continuous_variables(), initialPt); // view->copy
  if (iteratorCommRank == 0) {
    // define paramsMsgLen
    if (stratIterDedMaster) {
      RealVector rv(param_set_len);
      MPIPackBuffer send_buffer;
      send_buffer << rv;
      paramsMsgLen = send_buffer.size();
    }
    // define resultsMsgLen
    if (stratIterDedMaster && worldRank == 0)
      userDefinedModel.estimate_message_lengths(); // init_comms not called
    resultsMsgLen = userDefinedModel.message_lengths()[3];
  }

  // -------------------------------------------------------------------------
  // Define parameterSets from the combination of user-specified & random jobs
  // -------------------------------------------------------------------------
  if ( (  stratIterDedMaster && worldRank        == 0 ) ||
       ( !stratIterDedMaster && iteratorCommRank == 0 ) ) {

    // user-specified jobs
    copy_data(raw_param_sets, parameterSets, 0, param_set_len);

    // random jobs
    if (num_random_jobs) { // random jobs specified
      size_t i, j;
      RealVectorArray random_jobs;
      if (worldRank == 0) {
	// set up bounds for uniform sampling
	RealVector lower_bnds, upper_bnds;
	if (multiStartFlag) {
	  lower_bnds = userDefinedModel.continuous_lower_bounds(); // view OK
	  upper_bnds = userDefinedModel.continuous_upper_bounds(); // view OK
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
        int seed = problem_db.get_int("strategy.concurrent.random_seed");
	String sample_type, rng; // empty strings: use defaults
	NonDLHSSampling lhs_sampler(sample_type, num_random_jobs, seed, rng,
				    lower_bnds, upper_bnds);
	const RealMatrix& all_samples = lhs_sampler.all_samples();
	random_jobs.resize(num_random_jobs);
	for (i=0; i<num_random_jobs; ++i)
	  copy_data(all_samples[i], param_set_len, random_jobs[i]);
      }

      if (!stratIterDedMaster && numIteratorServers > 1) {
	// For static scheduling, bcast all random jobs over si_intra_comm (not 
	// necessary for self-scheduling as jobs are assigned from the master).
	if (worldRank == 0) {
	  MPIPackBuffer send_buffer;
	  send_buffer << random_jobs;
	  int buffer_len = send_buffer.size();
	  parallelLib.bcast_si(buffer_len);
	  parallelLib.bcast_si(send_buffer);
	}
	else {
	  int buffer_len;
	  parallelLib.bcast_si(buffer_len);
	  MPIUnpackBuffer recv_buffer(buffer_len);
	  parallelLib.bcast_si(recv_buffer);
	  recv_buffer >> random_jobs;
	}
      }

      // rescale (if needed) and append to parameterSets
      size_t cntr = parameterSets.size();
      parameterSets.resize(numIteratorJobs);
      for (i=0; i<num_random_jobs; ++i) {
        if (multiStartFlag)
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
    // distinction between a multi-start strategy and a sequential hybrid
    // strategy (initiated with global sampling).
  }

  // all iterator masters bookkeep on the full results list, even if
  // only some entries are defined locally
  if (iteratorCommRank == 0)
    prpResults.resize(numIteratorJobs);
}


ConcurrentStrategy::~ConcurrentStrategy()
{
  // Virtual destructor handles referenceCount at Strategy level.

  // Free the communicators once for all selectedIterator executions.
  // The strategy dedicated master processor is excluded.
  if (!stratIterDedMaster || worldRank)
    free_iterator(selectedIterator, userDefinedModel);
}


void ConcurrentStrategy::run_strategy()
{
  if (worldRank == 0) {
    Cout << "Running Concurrent Iterator Strategy..." << std::endl;
    if (!stratIterDedMaster) // first cut should work serially
      // set up plots and tabular data file
      selectedIterator.initialize_graphics(graph2DFlag, tabularDataFlag,
					   tabularDataFile);
  }

  schedule_iterators(selectedIterator, userDefinedModel);

  if (worldRank == 0) {
    Cout << "\n<<<<< Concurrent iteration completed.\n";
    print_results();
  }
}


void ConcurrentStrategy::print_results() const
{
  using std::setw;
  Cout << "\n<<<<< Results summary:\n";

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
  Cout << "   set_id "; // matlab comment syntax
  for (i=0; i<param_set_len; ++i) {
    if (multiStartFlag)
      Cout << setw(14) << cv_labels[i].data() << ' ';
    else {
      char string[10];
      std::sprintf(string, "w%i", (int)i + 1);
      Cout << setw(14) << string << ' ';
    }
  }
  for (i=0; i<num_cv; i++) {
    String label = (multiStartFlag) ? cv_labels[i] + String("*") : cv_labels[i];
    Cout << setw(14) << label.data() << ' ';
  }
  for (i=0; i<num_div; i++) {
    String label = (multiStartFlag) ?
      div_labels[i] + String("*") : div_labels[i];
    Cout << setw(14) << label.data() << ' ';
  }
  for (i=0; i<num_drv; i++) {
    String label = (multiStartFlag) ?
      drv_labels[i] + String("*") : drv_labels[i];
    Cout << setw(14) << label.data() << ' ';
  }
  for (i=0; i<num_fns; i++)
    Cout << setw(14) << fn_labels[i].data() << ' ';
  Cout << '\n';

  // Table data:
  size_t num_results = prpResults.size();
  for (i=0; i<num_results; ++i) {
    const ParamResponsePair& prp_result = prpResults[i];
    Cout << std::setprecision(10) << std::resetiosflags(std::ios::floatfield)
         << setw(9) << prp_result.eval_id() << ' ';
    for (size_t j=0; j<param_set_len; ++j)
      Cout << setw(14) << parameterSets[i][j] << ' ';
    const Variables& prp_vars = prp_result.prp_parameters();
    //prp_vars.write_tabular(Cout) not used since active vars, not all vars
    write_data_tabular(Cout, prp_vars.continuous_variables());
    write_data_tabular(Cout, prp_vars.discrete_int_variables());
    write_data_tabular(Cout, prp_vars.discrete_real_variables());
    prp_result.prp_response().write_tabular(Cout);
  }
  Cout << '\n';
}

} // namespace Dakota
