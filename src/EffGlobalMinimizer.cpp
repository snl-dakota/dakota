/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "EffGlobalMinimizer.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "NonDLHSSampling.hpp"
#include "RecastModel.hpp"
#include "DataFitSurrModel.hpp"
#include "DakotaApproximation.hpp"
#include "DakotaSurrogatesGP.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaGraphics.hpp"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NormalRandomVariable.hpp"
#include <string>

//#define DEBUG
//#define DEBUG_PLOTS

namespace Dakota {

EffGlobalMinimizer* EffGlobalMinimizer::effGlobalInstance(NULL);


EffGlobalMinimizer::
EffGlobalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedMinimizer(problem_db, model,
		     std::shared_ptr<TraitsBase>(new EffGlobalTraits())),
  batchSize(probDescDB.get_int("method.batch_size")),
  batchSizeExploration(probDescDB.get_int("method.batch_size.exploration")),
  dataOrder(1), batchEvalId(1),
  batchAsynch(probDescDB.get_short("method.synchronization") ==
	      NONBLOCKING_SYNCHRONIZATION)
{
  // substract the total batchSize from batchSizeExploration
  batchSizeAcquisition = batchSize - batchSizeExploration;

  // historical default convergence tolerances
  if (convergenceTol < 0.) convergenceTol = 1.e-12;
  distanceTol = probDescDB.get_real("method.x_conv_tol");
  if (distanceTol < 0.) distanceTol = 1.e-8;

  bestVariablesArray.push_back(iteratedModel.current_variables().copy());
  initialize_multipliers();

  // Always build a global Gaussian process model.  No correction is needed.
  String approx_type;
  switch (probDescDB.get_short("method.nond.emulator")) {
  case GP_EMULATOR:     approx_type = "global_gaussian";        break;
  case EXPGP_EMULATOR:  approx_type = "global_exp_gauss_proc";  break;
  default:              approx_type = "global_kriging";         break;
  }

  int db_samples = probDescDB.get_int("method.samples");
  int samples = (db_samples > 0) ? db_samples :
    (numContinuousVars+1)*(numContinuousVars+2)/2;
  // get point samples file
  const String& import_pts_file
    = probDescDB.get_string("method.import_build_points_file");
  String sample_reuse;
  if (!import_pts_file.empty()) // TO DO: allow reuse separate from import
    { samples = 0; sample_reuse = "all"; }
  else sample_reuse = "none";

  initialize_sub_problem(approx_type, samples,
			 probDescDB.get_int("method.random_seed"),
			 probDescDB.get_bool("method.derivative_usage"),
			 sample_reuse, import_pts_file,
			 probDescDB.get_ushort("method.import_build_format"),
			 probDescDB.get_bool("method.import_build_active_only"),
			 probDescDB.get_string(
			   "method.export_approx_points_file"),
			 probDescDB.get_ushort("method.export_approx_format"));

  if (approx_type == "global_exp_gauss_proc") {
#ifdef HAVE_DAKOTA_SURROGATES
    const String& advanced_options_file
      = problem_db.get_string("method.advanced_options_file");
    if (!advanced_options_file.empty())
      set_model_gp_options(fHatModel, advanced_options_file);
#else
    Cerr << "\nError: efficient_global does not support global_exp_gauss_proc "
         << "when Dakota is built without DAKOTA_MODULE_SURROGATES enabled." << std::endl;
    abort_handler(METHOD_ERROR);
#endif
  }
}


EffGlobalMinimizer::
EffGlobalMinimizer(Model& model, const String& approx_type, int samples,
		   int seed, bool use_derivs, size_t max_iter, size_t max_eval,
		   Real conv_tol):
  SurrBasedMinimizer(model, max_iter, max_eval, conv_tol,
		     std::shared_ptr<TraitsBase>(new EffGlobalTraits())),
  batchSize(1), batchSizeExploration(0), dataOrder(1), batchEvalId(1),
  batchAsynch(false)
{
  methodName = EFFICIENT_GLOBAL;

  // substract the total batchSize from batchSizeExploration
  batchSizeAcquisition = batchSize - batchSizeExploration;

  // historical default convergence tolerances
  //if (convergenceTol < 0.)
    convergenceTol = 1.e-12;
  //distanceTol = probDescDB.get_real("method.x_conv_tol");
  //if (distanceTol < 0.)
    distanceTol = 1.e-8;

  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  initialize_multipliers();
  initialize_sub_problem(approx_type, samples, seed, use_derivs, "none");
}


void EffGlobalMinimizer::initialize_multipliers()
{
  // initialize augmented Lagrange multipliers
  size_t i, num_multipliers = numNonlinearEqConstraints;
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    if (origNonlinIneqLowerBnds[i] > -bigRealBoundSize) // g has a lower bound
      ++num_multipliers;
    if (origNonlinIneqUpperBnds[i] <  bigRealBoundSize) // g has an upper bound
      ++num_multipliers;
  }
  augLagrangeMult.resize(num_multipliers);
  augLagrangeMult = 0.;
}


void EffGlobalMinimizer::
initialize_sub_problem(const String& approx_type, int samples, int seed,
		       bool use_derivs, const String& sample_reuse,
		       const String& import_build_points_file,
		       unsigned short import_build_format,
		       bool import_build_active_only,
		       const String& export_approx_points_file,
		       unsigned short export_approx_format)
{
  UShortArray approx_order; // empty
  short corr_order = -1, corr_type = NO_CORRECTION;
  if (use_derivs) {
    if (approx_type == "global_gaussian") {
      Cerr << "\nError: efficient_global does not support gaussian_process "
	   << "when derivatives present; use kriging instead." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    if (iteratedModel.gradient_type() != "none") dataOrder |= 2;
    if (iteratedModel.hessian_type()  != "none") dataOrder |= 4;
  }

  unsigned short sample_type = SUBMETHOD_DEFAULT;
  String rng; // empty string: use default
  bool vary_pattern = false;// for consistency across any outer loop invocations

  Iterator dace_iterator;
  dace_iterator.assign_rep(std::make_shared<NonDLHSSampling>(iteratedModel,
    sample_type, samples, seed, rng, vary_pattern, ACTIVE_UNIFORM));
  dace_iterator.active_set_request_values(dataOrder);

  // Construct f-hat (fHatModel) using a GP approximation for each response
  // function over the active/design vars (same view as iteratedModel:
  // not the typical All view for DACE).
  //const Variables& curr_vars = iteratedModel.current_variables();
  ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
  gp_set.request_values(1); // no surr deriv evals, but GP may be grad-enhanced
  const ShortShortPair& gp_view = iteratedModel.current_variables().view();
  fHatModel.assign_rep(std::make_shared<DataFitSurrModel>(dace_iterator,
    iteratedModel, gp_set, gp_view, approx_type, approx_order, corr_type,
    corr_order, dataOrder, outputLevel, sample_reuse, import_build_points_file,
    import_build_format, import_build_active_only, export_approx_points_file,
    export_approx_format));

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level at run time.
  SizetArray recast_vars_comps_total; // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  short recast_resp_order = 1; // nongradient-based optimizers
  approxSubProbModel.assign_rep(std::make_shared<RecastModel>(fHatModel,
    recast_vars_comps_total, all_relax_di, all_relax_dr,
    iteratedModel.current_variables().view(), 1, 0, 0, recast_resp_order));

  // must use alternate NoDB ctor chain
  size_t max_iter = 10000, max_eval = 50000;
  double min_box_size = 1.e-15, vol_box_size = 1.e-15;
#ifdef HAVE_NCSU
  approxSubProbMinimizer.assign_rep(std::make_shared<NCSUOptimizer>(
    approxSubProbModel, max_iter, max_eval, min_box_size, vol_box_size));
#else
  Cerr << "NCSU DIRECT is not available to optimize the GP subproblems. "
       << "Aborting process." << std::endl;
  abort_handler(METHOD_ERROR);
#endif //HAVE_NCSU

  // IteratorScheduler::init_iterator() initializes the parallel configuration
  // for EffGlobalMinimizer + iteratedModel using EffGlobalMinimizer's
  // maxEvalConcurrency.  During fHatModel construction above,
  // DataFitSurrModel::derived_init_communicators() initializes the parallel
  // config for dace_iterator + iteratedModel using dace_iterator's
  // maxEvalConcurrency.  The only iteratedModel concurrency currently exercised
  // is that used by dace_iterator within the initial GP construction, but the
  // EffGlobalMinimizer maxEvalConcurrency must still be set so as to avoid
  // parallel config errors resulting from avail_procs > max_concurrency within
  // IteratorScheduler::init_iterator().  A max of the local derivative
  // concurrency and the DACE concurrency is used for this purpose.
  maxEvalConcurrency = std::max(maxEvalConcurrency,
				dace_iterator.maximum_evaluation_concurrency());
}


void EffGlobalMinimizer::pre_run()
{
  //Minimizer::pre_run(); // invoke any base class definition (none defined)

  if (!approxSubProbModel.mapping_initialized()) {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    /*bool var_size_changed =*/ approxSubProbModel.initialize_mapping(pl_iter);
    //if (var_size_changed) resize();
  }

  // assign parallelFlag based on user spec and model asynch support
  check_parallelism();
  // initialize convergence counters and limits
  initialize_counters_limits(); // order dependency: requires parallelFlag
}


void EffGlobalMinimizer::core_run()
{
  EffGlobalMinimizer* prev_instance = effGlobalInstance;
  effGlobalInstance = this; // instance pointer used within static member fns

  //if (setUpType=="model") {

    // build initial GP for all response functions
    build_gp(); // TO DO: consider moving to pre_run() to enable alt workflow

    // iteratively adapt the GP (in parallel) until EGO converges
    if (batchAsynch) batch_asynchronous_ego();
    else             batch_synchronous_ego();

  //}
  //else if (setUpType=="user_functions") {
  //  Cerr << "Error: user_functions mode not implemented in EffGlobalMinimizer"
  // 	   << "::core_run()." << std::endl;
  //  abort_handler(METHOD_ERROR);
  //}
  //else {
  //  Cerr << "Error: bad setUpType in EffGlobalMinimizer::core_run()."
  // 	   << std::endl;
  //  abort_handler(METHOD_ERROR);
  //}

  effGlobalInstance = prev_instance;  // restore in case of recursion
}


void EffGlobalMinimizer::post_run(std::ostream& s)
{
  retrieve_final_results();

  if (approxSubProbModel.mapping_initialized())
    approxSubProbModel.finalize_mapping();

  Minimizer::post_run(s);
}


void EffGlobalMinimizer::check_parallelism()
{
  // Add safeguard: If model does not support asynchronous evals, then reset
  // batch sizes for serial execution and echo a warning

  if (batchSize > 1) {
    if (iteratedModel.asynch_flag())
      parallelFlag = true; // turn parallelFlag on; batchAsynch from user spec
    else { // revert to serial EGO settings
      Cerr << "Warning: concurrent operations not supported by model. "
	   << "Batch size request ignored." << std::endl;
      parallelFlag = batchAsynch = false;
      batchSize = batchSizeAcquisition = 1; batchSizeExploration = 0;
    }
  }
  else
    parallelFlag = batchAsynch = false;
}


void EffGlobalMinimizer::build_gp()
{
  // now that variables/labels/bounds/targets have flowed down at run-time from
  // any higher level recursions, propagate them up the instantiate-on-the-fly
  // Model recursion so that they are correct when they propagate back down.
  approxSubProbModel.update_from_subordinate_model(); // depth = max

  // (We might want a more selective update from submodel, or make a new
  // specialization of RecastModel.)  Always want to minimize the negative
  // expected improvement as posed in the EIF, which consumes min/max sense
  // and weights, and recasts nonlinear constraints, so we don't let these
  // propagate to the approxSubproblemMinimizer.
  approxSubProbModel.primary_response_fn_sense(BoolDeque());
  approxSubProbModel.primary_response_fn_weights(RealVector(), false);//no recur
  approxSubProbModel.user_defined_constraints().reshape_nonlinear(0, 0);

  // Build initial GPs for all response functions
  if (batchAsynch)
    fHatModel.track_evaluation_ids(true); // enable replacements by eval id
  fHatModel.build_approximation();
  // initialize counter for GP refinements (used for vars{Acq,Expl}Map)
  batchEvalId = iteratedModel.evaluation_id() + 1;
}


void EffGlobalMinimizer::batch_synchronous_ego()
{
  while (!converged()) {

    // construct the acquisition batch
    construct_batch_acquisition(batchSizeAcquisition, batchSize);
    // construct the exploration batch
    construct_batch_exploration(batchSizeExploration, batchSize);

    // blocking synch for composite batch (acquisition + exploration)
    evaluate_batch(true); // rebuild
  }
}


void EffGlobalMinimizer::batch_asynchronous_ego()
{
  size_t new_acq, new_expl, new_batch;
  while (!converged()) {

    // non-blocking synch for composite batch (acquisition + exploration)
    /*bool completed = */query_batch(true); // rebuild
    //if (globalIterCount && !completed) delay();

    // If new jobs are allocated based on batch_ratio * total_completed, the
    // common case of one completion always gets assigned to the larger of
    // the two batch sizes, starving the other.  Therefore, keep two queues
    // and backfill based on type of completed job.
    new_acq   = batchSizeAcquisition - varsAcquisitionMap.size();
    new_expl  = batchSizeExploration - varsExplorationMap.size();
    new_batch = new_acq + new_expl;

    // construct the acquisition batch
    construct_batch_acquisition(new_acq,  new_batch);
    // construct the exploration batch
    construct_batch_exploration(new_expl, new_batch);

    // launch new truth jobs using liar variable sets
    backfill_batch(new_acq, new_expl);
  }

  // Complete any jobs that are still running at time of convergence kick out.
  // Don't rebuild as only need the final build data for extract_best_sample().
  while (!empty_queues())
    /*bool completed = */query_batch(false); // no rebuild
    //if (!completed) delay();
}


void EffGlobalMinimizer::
construct_batch_acquisition(size_t new_acq, size_t new_batch)
{
  if (!new_acq) return;

  // initialize EIF recast model
  Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
  primary_resp_map[0].resize(numFunctions);
  size_t i;
  for (i=0; i<numFunctions; i++)
    primary_resp_map[0][i] = i;
  BoolDequeArray nonlinear_resp_map(1, BoolDeque(numFunctions, true));

  // set objective ptr for asp_model_rep to EIF_objective_eval for all i
  std::shared_ptr<RecastModel> asp_model_rep
    = std::static_pointer_cast<RecastModel>(approxSubProbModel.model_rep());
  asp_model_rep->init_maps(vars_map, false, NULL, NULL, primary_resp_map,
    secondary_resp_map, nonlinear_resp_map, EIF_objective_eval, NULL);

  // construct the acquisition batch
  for (i=0; i<new_acq; ++i, ++batchEvalId) {

    Cout << "\n>>>>> Initiating global iteration " << ++globalIterCount
	 << " (acquisition batch " << i+1 << ")\n";

    // determine meritFnStar for use in EIF
    compute_best_sample();

    // execute GLOBAL search and retrieve results
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    approxSubProbMinimizer.reset();
    approxSubProbMinimizer.run(pl_iter); // maximize the EI acquisition fn
    const Variables&   vars_star = approxSubProbMinimizer.variables_results();
    const Response& ei_resp_star = approxSubProbMinimizer.response_results();

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nResults of EGO iteration:\nFinal point =\n" << vars_star
	   << "Expected Improvement    =\n" << std::setw(write_precision+28)
	   << -ei_resp_star.function_value(0) << '\n';

    // For acquisition, we monitor history of vars* and EIF* (both deactivated
    // for exploration)
    update_convergence_counters(vars_star, ei_resp_star);

    // append liar in parallel mode, even if it will be replaced before the next
    // approx sub-problem solve (cost does not justify increased complexity in
    // replace/pop logic).  But do suppress an unnecessary rebuild if last
    // look-ahead before truth synchronization, since this can be expensive.
    if (parallelFlag) {
      bool rebuild = (new_batch > new_acq || i+1 < new_acq);
      append_liar(vars_star, batchEvalId, rebuild);
    }

    // save a copy for truth replacement downstream
    varsAcquisitionMap[batchEvalId] = vars_star.copy();
  }
}


void EffGlobalMinimizer::
construct_batch_exploration(size_t new_expl, size_t new_batch)
{
  if (!new_expl) return;

  // initialize EIF recast model
  Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
  size_t i;
  primary_resp_map[0].resize(numFunctions);
  for (i=0; i<numFunctions; i++)
    primary_resp_map[0][i] = i;
  BoolDequeArray nonlinear_resp_map(1, BoolDeque(numFunctions, true));

  // set objective ptr for asp_model_rep to Variances_objective_eval for all i
  std::shared_ptr<RecastModel> asp_model_rep
    = std::static_pointer_cast<RecastModel>(approxSubProbModel.model_rep());
  asp_model_rep->init_maps(vars_map, false, NULL, NULL, primary_resp_map,
    secondary_resp_map, nonlinear_resp_map, Variances_objective_eval, NULL);

  // construct the exploration batch
  for (i=0; i<new_expl; ++i, ++batchEvalId) {

    Cout << "\n>>>>> Initiating global iteration " << ++globalIterCount
	 << " (exploration batch " << i+1 << ")\n";
    
    // execute GLOBAL search and retrieve results
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    approxSubProbMinimizer.reset();
    approxSubProbMinimizer.run(pl_iter); // maximize the posterior variance fn
    const Variables& vars_star = approxSubProbMinimizer.variables_results();

    if (outputLevel >= NORMAL_OUTPUT) {
      const Response& pv_resp_star = approxSubProbMinimizer.response_results();
      Real pv_star = -pv_resp_star.function_value(0);
      Cout << "\nResults of EGO iteration:\nFinal point =\n" << vars_star
	   << "Prediction Variance     =\n                     "
	   << std::setw(write_precision+7) << pv_star << '\n';
    }

    // We do not monitor value of pv_resp_star as a convergence counter.
    // We assume the user's intent is to augment acquisition with some optional
    // exploration, where convergence is only achieved based on exploiting
    // good solutions and not based on a lack of good exploration candidates.
    // Similarly, don't update prevSubProbSoln coming from exploration.
    //update_convergence_counters(vars_star);//, pv_resp_star);
    
    // append liar in parallelMode, even if it will be replaced before the next
    // approx sub-problem solve (cost does not justify increased complexity in
    // replace/pop logic).  But do suppress an unnecessary rebuild if last
    // look-ahead before truth synchronization, since this can be expensive.
    if (parallelFlag) {
      bool rebuild = (i+1 < new_expl);
      append_liar(vars_star, batchEvalId, rebuild);
    }

    // save a copy for truth replacement downstream
    varsExplorationMap[batchEvalId] = vars_star.copy();
  }
}


void EffGlobalMinimizer::
append_liar(const Variables& vars_star, int liar_id, bool rebuild)
{
  // get approximate (liar) response value for optimal point (vars_star)
  fHatModel.current_variables().active_variables(vars_star);
  fHatModel.evaluate();
  const Response& fhat_resp_star = fHatModel.current_response();

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "                     " << std::setw(write_precision+7)
	 << augmented_lagrangian(fhat_resp_star.function_values())
	 << " [approx merit]\n";

  // update GP by appending constant liar to fHatModel (aka heuristic liar)
  // > Do not update constraint penalties/multipliers based on liar data, as
  //   these accumulate increments --> only accumulate once for truth data
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nParallel EGO: appending liar response for evaluation "
	 << liar_id << ".\n";
  IntResponsePair liar_resp_pr(liar_id, fhat_resp_star);
  fHatModel.append_approximation(vars_star, liar_resp_pr, rebuild);
  //numDataPts = fHatModel.approximation_data(0).points(); // updated count
}


void EffGlobalMinimizer::evaluate_batch(bool rebuild)
{
  fHatModel.component_parallel_mode(TRUTH_MODEL_MODE);
  if (parallelFlag) {

    // remove all liar responses prior to appending truth
    pop_liar_responses(); // Note: replace could avoid sdv pop + re-append

    // launch all jobs defined in batch and block on their completion
    launch_batch();
    const IntResponseMap& truth_resp_map = iteratedModel.synchronize();

    // update the GP approximation with batch results
    // reuse varsAcquisitionMap for composite map to avoid extra copies, as
    // will be cleared at bottom.  Could also generalize append_approximation
    // to advance both maps and only complete matches within the id sequences
    // (and call for each varsMap), but adds complexity with little gain.
    varsAcquisitionMap.insert(varsExplorationMap.begin(),
			      varsExplorationMap.end());
    fHatModel.append_approximation(varsAcquisitionMap, truth_resp_map, rebuild);

    // update constraints (truth resp only, not for liar resp)
    if (numNonlinearConstraints)
      update_constraints(truth_resp_map);
  }
  else {
    // no pop: liar was not appended in serial case

    IntVarsMCIter v_cit = (varsAcquisitionMap.empty()) ?
      --varsExplorationMap.end() : --varsAcquisitionMap.end();
    const Variables& vars_star = v_cit->second;
    launch_single(vars_star);

    // update the GP approximation
    const Response& truth_resp = iteratedModel.current_response();
    IntResponsePair truth_resp_pr(iteratedModel.evaluation_id(), truth_resp);
    fHatModel.append_approximation(vars_star, truth_resp_pr, rebuild);

    // update constraints (truth resp only, not for liar resp)
    if (numNonlinearConstraints)
      update_constraints(truth_resp.function_values());
  }

  varsAcquisitionMap.clear();  varsExplorationMap.clear();
}


/** query running jobs and process any new completions */
bool EffGlobalMinimizer::query_batch(bool rebuild)
{
  if (empty_queues()) return false;

  // nonblocking synchronize: evaluate truth responses in parallel
  fHatModel.component_parallel_mode(TRUTH_MODEL_MODE);
  const IntResponseMap& truth_resp_map = iteratedModel.synchronize_nowait();
  if (truth_resp_map.empty()) return false;

  process_truth_response_map(truth_resp_map, rebuild);
  update_variable_maps(truth_resp_map);
  return true;
}


void EffGlobalMinimizer::backfill_batch(size_t new_acq, size_t new_expl)
{
  if (!new_acq && !new_expl) return;

  // queue nonblocking evals for composite batch (acquisition + exploration),
  // launching the trailing map id's in the sequence defined by batchEvalId
  ActiveSet set = iteratedModel.current_response().active_set();
  set.request_values(dataOrder);
  IntVarsMCIter a_cit = varsAcquisitionMap.begin(),
                e_cit = varsExplorationMap.begin();
  std::advance(a_cit, varsAcquisitionMap.size() - new_acq);
  std::advance(e_cit, varsExplorationMap.size() - new_expl);
  int a_id = extract_id(a_cit, varsAcquisitionMap),
      e_id = extract_id(e_cit, varsExplorationMap);
  while (a_id != INT_MAX || e_id != INT_MAX) {
    // properly sequence backfill evaluations across the two queues so that
    // the liar/truth sequences are synchronized, enabling id-based replacement
    if (a_id < e_id) {
      iteratedModel.current_variables().active_variables(a_cit->second);
      iteratedModel.evaluate_nowait(set);
      a_id = extract_id(++a_cit, varsAcquisitionMap);
    }
    else if (e_id < a_id) {
      iteratedModel.current_variables().active_variables(e_cit->second);
      iteratedModel.evaluate_nowait(set);
      e_id = extract_id(++e_cit, varsExplorationMap);
    }
    else {
      Cerr << "Error: duplicate evaluation ids in EffGlobalMinimizer::"
	   << "backfill_batch()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
}


void EffGlobalMinimizer::launch_batch()
{
  // queue evaluations for composite batch (acquisition + exploration)
  ActiveSet set = iteratedModel.current_response().active_set();
  set.request_values(dataOrder);
  IntVarsMIter v_it;
  for (v_it =varsAcquisitionMap.begin();
       v_it!=varsAcquisitionMap.end(); ++v_it) {
    iteratedModel.current_variables().active_variables(v_it->second);
    iteratedModel.evaluate_nowait(set);
  }
  for (v_it =varsExplorationMap.begin();
       v_it!=varsExplorationMap.end(); ++v_it) {
    iteratedModel.current_variables().active_variables(v_it->second);
    iteratedModel.evaluate_nowait(set);
  }
}


void EffGlobalMinimizer::launch_single(const Variables& vars_star)
{
  // serial evaluation
  iteratedModel.current_variables().active_variables(vars_star);
  ActiveSet set = iteratedModel.current_response().active_set();
  set.request_values(dataOrder);
  iteratedModel.evaluate(set);
}


void EffGlobalMinimizer::
process_truth_response_map(const IntResponseMap& truth_resp_map, bool rebuild)
{
  if (truth_resp_map.empty()) return;

  // Process completions: replace liar resp w/ new truth resp based on eval ids
  fHatModel.replace_approximation(truth_resp_map, rebuild);
  // update constraints (truth resp only, not for liar resp)
  if (numNonlinearConstraints)
    update_constraints(truth_resp_map);
}


void EffGlobalMinimizer::
update_variable_maps(const IntResponseMap& truth_resp_map)
{
  // Remove completed evals from varMaps (using single iterator traversals
  // rather than repeated lookups)
  IntVarsMIter a_it = varsAcquisitionMap.begin(),
               e_it = varsExplorationMap.begin();
  int r_id, a_id = extract_id(a_it, varsAcquisitionMap),
            e_id = extract_id(e_it, varsExplorationMap);
  IntRespMCIter r_cit;
  for (r_cit=truth_resp_map.begin(); r_cit!=truth_resp_map.end(); ++r_cit) {
    r_id = r_cit->first;
    while (a_id < r_id)
      a_id = extract_id(++a_it, varsAcquisitionMap);
    while (e_id < r_id)
      e_id = extract_id(++e_it, varsExplorationMap);
    // Note: use postfix iterator increments to avoid invalidation by erase
    if (a_id == r_id) {
      varsAcquisitionMap.erase(a_it++); // copy a_it, increment orig, erase copy
      a_id = extract_id(a_it, varsAcquisitionMap);
    }
    else if (e_id == r_id) {
      varsExplorationMap.erase(e_it++); // copy e_it, increment orig, erase copy
      e_id = extract_id(e_it, varsExplorationMap);
    }
    else {
      Cerr << "Error: no match for response id in EffGlobalMinimizer::"
	   << "query_batch()" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
}


/** Extract the best merit function from build data through evaluaton
    of points on fHatModel.  This merit fn value is used within the
    EIF during an approximate sub-problem solve. */
void EffGlobalMinimizer::compute_best_sample()
{
  // pull the samples and responses from data used to build latest GP
  // to determine meritFnStar for use in the expected improvement function

  const Pecos::SurrogateData& gp_data_0 = fHatModel.approximation_data(0);
  const Pecos::SDVArray&    sdv_array_0 = gp_data_0.variables_data();
  size_t i, index_star = 0, num_data_pts = gp_data_0.points();
  Real merit_fn;  meritFnStar = DBL_MAX;
  RealVector fn_sample(numFunctions);
  for (i=0; i<num_data_pts; ++i) {

    const RealVector& cv = sdv_array_0[i].continuous_variables();

    fHatModel.current_variables().continuous_variables(cv);
    fHatModel.evaluate();
    const RealVector& f_hat = fHatModel.current_response().function_values();
    merit_fn = augmented_lagrangian(f_hat);

    if (merit_fn < meritFnStar)
      { index_star = i;  meritFnStar = merit_fn; }
  }

  // Only meritFnStar required for EIF in approx sub-problem solve:
  //copy_data(sdv_array_0[index_star].continuous_variables(), cVarsStar);
  //extract_qoi_build_data(index_star, truthFnStar);
}


/** Extract the best point from the build data for final results reporting. */
void EffGlobalMinimizer::extract_best_sample()
{
  // pull the samples and responses from data used to build latest GP
  // to determine final cVarsStar and truthFnStar

  const Pecos::SurrogateData& gp_data_0 = fHatModel.approximation_data(0);
  size_t i, index_star = 0, num_data_pts = gp_data_0.points();
  Real merit_fn, merit_fn_star = DBL_MAX;
  RealVector fn_sample(numFunctions);
  for (i=0; i<num_data_pts; ++i) {

    // extract build data from individual surrogates and form merit fn using
    // latest penalties/multipliers.
    // > this may include liar data for a particular look-ahead iteration, but
    //   it is rescanned from scratch each time using the most up-to-date data
    //   (any pollution is temporary and gets removed).
    extract_qoi_build_data(i, fn_sample);
    merit_fn = augmented_lagrangian(fn_sample);

    if (merit_fn < merit_fn_star)
      { index_star = i; merit_fn_star = merit_fn; }
  }

  // update best{Variables,Response}Array from index_star
  const Pecos::SDVArray& sdv_array_0 = gp_data_0.variables_data();
  const RealVector& cv_star = sdv_array_0[index_star].continuous_variables();
  bestVariablesArray.front().continuous_variables(cv_star);
  RealVector fn_star = bestResponseArray.front().function_values_view();
  extract_qoi_build_data(index_star, fn_star);
}


void EffGlobalMinimizer::
extract_qoi_build_data(size_t data_index, RealVector& fn_vals)
{
  if (fn_vals.length() != numFunctions)
    fn_vals.sizeUninitialized(numFunctions);
  // loop over QoI approximations, extracting the QoI value for the passed index
  for (size_t i=0; i<numFunctions; ++i) {
    const Pecos::SDRArray& sdr_array
      = fHatModel.approximation_data(i).response_data();
    fn_vals[i] = sdr_array[data_index].response_function();
  }
}


bool EffGlobalMinimizer::converged()
{ 
  // set convergence flag if any counters have reached their limits
  bool conv = ( distConvergenceCntr >= distConvergenceLimit ||
		eifConvergenceCntr  >= eifConvergenceLimit ||
		globalIterCount     >= maxIterations );

  if (conv || outputLevel >= DEBUG_OUTPUT) {
    if (distConvergenceCntr >= distConvergenceLimit)
      Cout << "\nStopping criteria met:     distConvergenceCntr ("
	   << distConvergenceCntr << ") >= ";
    else
      Cout << "\nStopping criteria not met: distConvergenceCntr ("
	   << distConvergenceCntr << ") < ";
    Cout << "distConvergenceLimit (" << distConvergenceLimit << ")\n";

    if (eifConvergenceCntr >= eifConvergenceLimit)
      Cout << "Stopping criteria met:     eifConvergenceCntr ("
	   << eifConvergenceCntr << ") >= ";
    else
      Cout << "Stopping criteria not met: eifConvergenceCntr ("
	   << eifConvergenceCntr << ") < ";
    Cout << "eifConvergenceLimit (" << eifConvergenceLimit << ")\n";

    if (globalIterCount >= maxIterations)
      Cout << "Stopping criteria met:     globalIterCount ("
	   << globalIterCount << ") >= ";
    else
      Cout << "Stopping criteria not met: globalIterCount ("
	   << globalIterCount << ") < ";
    Cout << "maxIterations (" << maxIterations << ")\n";
  }

  return conv;
}

// Some thoughts on convergence assessments: *** TO DO ***
// > For vars*, truth evaluation is irrelevant and EIF* is based on approx
//   subproblem solve, so nothing to modify/update with recovered truth evals.
// > We do need to factor in the relative state of the GP in terms of
//   truth/liar response content and discount convergence assessements
//   with signficiant liar accumulation.
//   >> for batch-synchronous, could simply use the vars*Map index to
//      indicate the amount of liar corruption.  Could also consider entirely
//      ignoring iterations with any liar data, but this is not an option for
//      batch-asynch where some amount of liar data might always be present.
//   >> for batch-asynchronous, this is trickier since a job near the front of
//      the variables queue could have been generated with considerable liar
//      corruption, even though the liar jobs in front of it have cleared out
//      (the state of the GP when the point was generated indicates the degree
//      of trust in that queued point)
//   >> Consider implementing an increment based on aggregate GP "health" (e.g.
//      truth/total build data) where batch-synch could use higher penalization.
//      E.g., real_cntr += GP_truth2total ^ p  for p ~= 2 (synch), 10 (asynch)
//            real_cntr += 2./(2.+vars_map_index),     etc.

void EffGlobalMinimizer::
update_convergence_counters(const Variables& vars_star)
{
  // check for convergence based in distance between successive points
  // If the dist between successive points is very small, then there is
  // little value in updating the GP since the new training point will
  // essentially be the previous optimal point.
  const RealVector& c_vars = vars_star.continuous_variables();
  Real dist_cv_star = (prevSubProbSoln.empty()) ? DBL_MAX
                    : rel_change_L2(c_vars, prevSubProbSoln);
  if (dist_cv_star < distanceTol) ++distConvergenceCntr; // increment
  else                              distConvergenceCntr = 0; // reset

  // update prevSubProbSoln
  copy_data(c_vars, prevSubProbSoln); // *** TO DO: distinguish truth vs. liar?

  if (outputLevel >= DEBUG_OUTPUT) {
    debug_print_values(vars_star);
    debug_print_dist_counters(dist_cv_star);
  }
}


void EffGlobalMinimizer::
update_convergence_counters(const Response& resp_star)
{
  Real eif_star = -resp_star.function_value(0);
  // Check for convergence based on max EIF
  // > Note: maximal EIF is based solely on approx subproblem
  // > *** TO DO: distinguish truth-only surrogate vs. liar-corrupted?
  if ( eif_star < convergenceTol ) ++eifConvergenceCntr; // increment
  else                               eifConvergenceCntr = 0; // reset

  // If DIRECT failed to find a point with EIF>0, it returns the center point
  // as the optimal solution. EGO may have converged, but DIRECT may have just
  // failed to find a point with a good EIF value.  Adding this midpoint can
  // alter the GPs enough to allow DIRECT to find something useful, so we force
  // max(EIF)<tol twice to make sure. Note that we cannot make this check more
  // than 2 because it would cause EGO to add the center point more than once,
  // which will damage the GPs.  Unfortunately, when it happens the second time,
  // it may still be that DIRECT failed and not that EGO converged.
  if (outputLevel >= DEBUG_OUTPUT) debug_print_eif_counters(eif_star);
}


void EffGlobalMinimizer::retrieve_final_results()
{
  // Set best variables and response for use by strategy level.
  extract_best_sample(); // pull optimal result from sample data

  // (conditionally) export final surrogates
  export_final_surrogates(fHatModel);

  debug_plots(); // only plotting final (moot with deprecated graphics)
}

////////////////  MIKE TO EDIT ABOVE; TOM AND ANH BELOW //////////////////

/** To maximize expected improvement (PI), the approxSubProbMinimizer
    will minimize -(compute_probability_improvement). **/
void EffGlobalMinimizer::
PIF_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // Implementation of PI acquisition function
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = effGlobalInstance->fHatModel.approximation_variances(recast_vars);
  const ShortArray& recast_asv = recast_response.active_set_request_vector();

  if (recast_asv[0] & 1) { // return -EI since we are maximizing
    Real neg_pi
      = - effGlobalInstance->compute_probability_improvement(means, variances);
    recast_response.function_value(neg_pi, 0);
  }
}


/** To maximize expected improvement (EI), the approxSubProbMinimizer
    will minimize -(compute_expected_improvement). **/
void EffGlobalMinimizer::
EIF_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // Implementation of EI acquisition function
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = effGlobalInstance->fHatModel.approximation_variances(recast_vars);
  const ShortArray& recast_asv = recast_response.active_set_request_vector();

  if (recast_asv[0] & 1) { // return -EI since we are maximizing
    Real neg_ei
      = - effGlobalInstance->compute_expected_improvement(means, variances);
    recast_response.function_value(neg_ei, 0);
  }
}


/** To maximize lower confidence bound (LCB), the approxSubProbMinimizer
    will minimize -(compute_lower_confidence_bound). **/
void EffGlobalMinimizer::
LCB_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // Implementation of LCB acquisition function
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = effGlobalInstance->fHatModel.approximation_variances(recast_vars);
  const ShortArray& recast_asv = recast_response.active_set_request_vector();

  if (recast_asv[0] & 1) { // return -LCB since we are maximizing
    Real neg_lcb
      = - effGlobalInstance->compute_lower_confidence_bound(means, variances);
    recast_response.function_value(neg_lcb, 0);
  }
}


/** To maximize variances, the approxSubProbMinimizer will minimize
    -(variances). **/
void EffGlobalMinimizer::
Variances_objective_eval(const Variables& sub_model_vars,
			 const Variables& recast_vars,
			 const Response& sub_model_response,
			 Response& recast_response)
{
  // Implementation of MSE acquisition function
  // Means are passed in, but must retrieve variance from the GP
  // const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = effGlobalInstance->fHatModel.approximation_variances(recast_vars);
  const ShortArray& recast_asv = recast_response.active_set_request_vector();

  if (recast_asv[0] & 1) { // return -EI since we are maximizing
    Real neg_var = - effGlobalInstance->compute_variances(variances);
    recast_response.function_value(neg_var, 0);
  }
}


/** Compute the PI acquisition function **/
Real EffGlobalMinimizer::
compute_probability_improvement(const RealVector& means,
				const RealVector& variances)
{
  // Objective calculation will incorporate any sense changes or
  // weights, such that this is an objective to minimize.
  Real mean = objective(means, iteratedModel.primary_response_fn_sense(),
			iteratedModel.primary_response_fn_weights()), stdv;
  if ( numNonlinearConstraints ) {
    // mean_M = mean_f + lambda*EV + r_p*EV*EV
    // stdv_M = stdv_f
    const RealVector& ev = expected_violation(means, variances);
    for (size_t i=0; i<numNonlinearConstraints; ++i)
      mean += augLagrangeMult[i]*ev[i] + penaltyParameter*ev[i]*ev[i];
    stdv = std::sqrt(variances[0]); // ***
  }
  else { // extend for NLS/MOO ***
    // mean_M = M(mu_f)
    // stdv_M = sqrt(var_f)
    stdv = std::sqrt(variances[0]); // *** sqrt(sum(variances(1:nUsrPrimaryFns))
  }
  // Calculate the probability improvement
  Real cdf, snv = (meritFnStar - mean); // standard normal variate
  if (std::fabs(snv) >= std::fabs(stdv)*50.0)
    // this will trap the denominator=0.0 case even if numerator=0.0
    cdf = (snv > 0.) ? 1. : 0.;
  else
    cdf = Pecos::NormalRandomVariable::std_cdf(snv/stdv);

  return cdf;
}


/** Compute the EI acquisition function **/
Real EffGlobalMinimizer::
compute_expected_improvement(const RealVector& means,
			     const RealVector& variances)
{
  // Objective calculation will incorporate any sense changes or
  // weights, such that this is an objective to minimize.
  Real mean = objective(means, iteratedModel.primary_response_fn_sense(),
			iteratedModel.primary_response_fn_weights()), stdv;
  if ( numNonlinearConstraints ) {
    // mean_M = mean_f + lambda*EV + r_p*EV*EV
    // stdv_M = stdv_f
    const RealVector& ev = expected_violation(means, variances);
    for (size_t i=0; i<numNonlinearConstraints; ++i)
      mean += augLagrangeMult[i]*ev[i] + penaltyParameter*ev[i]*ev[i]; // ***
    stdv = std::sqrt(variances[0]); // *** if variance is only dependent on parameter points and not on QoI observations, then this would be Ok
  }
  else { // extend for NLS/MOO ***
    // mean_M = M(mu_f)
    // stdv_M = sqrt(var_f)
    stdv = std::sqrt(variances[0]); // *** sqrt(sum(variances(1:nUsrPrimaryFns))
  }
  // Calculate the expected improvement
  Real cdf, pdf;
  Real snv = (meritFnStar - mean); // standard normal variate
  if (std::fabs(snv) >= std::fabs(stdv)*50.0) {
    // this will trap the denominator=0.0 case even if numerator=0.0
    pdf = 0.;
    cdf = (snv > 0.) ? 1. : 0.;
  }
  else{
    snv /= stdv;
    cdf  = Pecos::NormalRandomVariable::std_cdf(snv);
    pdf  = Pecos::NormalRandomVariable::std_pdf(snv);
  }

  return (meritFnStar - mean) * cdf + stdv * pdf; // EI
}


/** Compute the LCB acquisition function **/
Real EffGlobalMinimizer::
compute_lower_confidence_bound(const RealVector& means,
			       const RealVector& variances)
{
  // Objective calculation will incorporate any sense changes or
  // weights, such that this is an objective to minimize.
  Real mean = objective(means, iteratedModel.primary_response_fn_sense(),
			iteratedModel.primary_response_fn_weights()), stdv;
  if ( numNonlinearConstraints ) {
    // mean_M = mean_f + lambda*EV + r_p*EV*EV
    // stdv_M = stdv_f
    const RealVector& ev = expected_violation(means, variances);
    for (size_t i=0; i<numNonlinearConstraints; ++i)
      mean += augLagrangeMult[i]*ev[i] + penaltyParameter*ev[i]*ev[i];
    stdv = std::sqrt(variances[0]); // ***
  }
  else { // extend for NLS/MOO ***
    // mean_M = M(mu_f)
    // stdv_M = sqrt(var_f)
    stdv = std::sqrt(variances[0]);// *** sqrt(sum(variances(1:numUserPrimary))
  }

  Real kappa = 2.; // in future, vary this parameter as a function of iterations
  return -mean + kappa * stdv; // lower confidence bound
}


/** Compute the variances **/
Real EffGlobalMinimizer::compute_variances(const RealVector& variances)
{
  // Objective calculation will incorporate any sense changes or
  // weights, such that this is an objective to minimize.
  return std::sqrt(variances[0]);// *** sqrt(sum(variances(1:numUserPrimaryFns))
}


/** Compute the expected violation for constraints **/
RealVector EffGlobalMinimizer::
expected_violation(const RealVector& means, const RealVector& variances)
{
  RealVector ev(numNonlinearConstraints);

  size_t i, cntr=0;
  // Inequality constraints
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    const Real& mean = means[numUserPrimaryFns+i];
    const Real& stdv = std::sqrt(variances[numUserPrimaryFns+i]);
    const Real& lbnd = origNonlinIneqLowerBnds[i];
    const Real& ubnd = origNonlinIneqUpperBnds[i];
    if (lbnd > -bigRealBoundSize) {
      Real cdf, pdf;
      Real snv = (lbnd-mean);
      if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
	pdf=0.0;
	cdf=(snv>0.0)?1.0:0.0;
      }
      else {
	snv/=stdv; //now snv is the standard normal variate
	cdf = Pecos::NormalRandomVariable::std_cdf(snv);
	pdf = Pecos::NormalRandomVariable::std_pdf(snv);
      }
      ev[cntr++] = (lbnd-mean)*cdf + stdv*pdf;
    }
    if (ubnd < bigRealBoundSize) {
      Real cdf, pdf;
      Real snv = (ubnd-mean);
      if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
	pdf=0.0;
	cdf=(snv>0.0)?1.0:0.0;
      }
      else {
	snv/=stdv;
	cdf = Pecos::NormalRandomVariable::std_cdf(snv);
	pdf = Pecos::NormalRandomVariable::std_pdf(snv);
      }
      ev[cntr++] = (mean-ubnd)*(1.-cdf) + stdv*pdf;
    }
  }

  // Equality constraints
  for (i=0; i<numNonlinearEqConstraints; i++) {
    const Real& mean = means[numUserPrimaryFns+numNonlinearIneqConstraints+i];
    const Real& stdv
      = std::sqrt(variances[numUserPrimaryFns+numNonlinearIneqConstraints+i]);
    const Real& zbar = origNonlinEqTargets[i];
    Real cdf, pdf;
    Real snv = (zbar-mean);
    if(std::fabs(snv)*50.0>=std::fabs(stdv)) {
      pdf=0.0;
      cdf=(snv>=0.0)?1.0:0.0;
    }
    else{
      snv/=stdv;
      cdf = Pecos::NormalRandomVariable::std_cdf(snv);
      pdf = Pecos::NormalRandomVariable::std_pdf(snv);
    }
    ev[cntr++] = (zbar-mean)*(2.*cdf-1.) + 2.*stdv*pdf;
  }

  return ev;
}


void EffGlobalMinimizer::update_penalty()
{
  // Logic follows Conn, Gould, and Toint, section 14.4, step 3
  //   CGT use mu *= tau with tau = 0.01 ->   r_p *= 50
  //   Rodriguez, Renaud, Watson:             r_p *= 10
  //   Robinson, Willcox, Eldred, and Haimes: r_p *= 5
  penaltyParameter *= 10.;
  //penaltyParameter = std::min(penaltyParameter, 1.e+20); // cap max penalty?
  Real mu = 1./2./penaltyParameter; // conversion between r_p and mu penalties
  etaSequence = eta * std::pow(mu, alphaEta);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Penalty updated: " << penaltyParameter << '\n'
	 << "eta updated:     " << etaSequence      << '\n'
	 << "Augmented Lagrange multipliers:\n" << augLagrangeMult;
}


void EffGlobalMinimizer::debug_plots()
{
#ifdef DEBUG_PLOTS
  // DEBUG - output set of samples used to build the GP
  // If problem is 2d, output a grid of points on the GP
  //   and truth (if requested)
  std::string samples_file, gp_file, var_file;
  for (size_t i=0; i<numFunctions; i++) {
    std::string tag = "_" + std::to_string(i+1) + ".out";
    samples_file = "ego_samples" + tag;
    std::ofstream s_out(samples_file.c_str(),std::ios::out);
    s_out << std::scientific;
    const Pecos::SurrogateData& gp_data = fHatModel.approximation_data(i);
    const Pecos::SDVArray& sdv_array = gp_data.variables_data();
    const Pecos::SDRArray& sdr_array = gp_data.response_data();
    size_t j, k, num_data_pts = gp_data.size(), num_vars = fHatModel.current_variables().cv();
    for (j=0; j<num_data_pts; ++j) {
      s_out << '\n';
      const RealVector& sample = sdv_array[j].continuous_variables();
      for (k=0; k<num_vars; k++)
	s_out << std::setw(13) << sample[k] << ' ';
      s_out << std::setw(13) << sdr_array[j].response_function();
    }
    s_out << std::endl;

    // Plotting the GP over a grid is intended for visualization and
    // is therefore only available for 2D problems
    if (num_vars==2) {
      gp_file  = "ego_gp"  + tag;
      var_file = "ego_var" + tag;
      std::ofstream  gp_out(gp_file.c_str(),  std::ios::out);
      std::ofstream var_out(var_file.c_str(), std::ios::out);
      std::ofstream eif_out("ego_eif.out",    std::ios::out);
      gp_out  << std::scientific;
      var_out << std::scientific;
      eif_out << std::scientific;
      RealVector test_pt(2);
      const RealVector& lbnd = fHatModel.continuous_lower_bounds();
      const RealVector& ubnd = fHatModel.continuous_upper_bounds();
      Real interval0 = (ubnd[0] - lbnd[0])/100.,
	   interval1 = (ubnd[1] - lbnd[1])/100.;
      for (j=0; j<101; j++) {
	test_pt[0] = lbnd[0] + float(j) * interval0;
	for (k=0; k<101; k++) {
	  test_pt[1] = lbnd[1] + float(k) * interval1;

	  fHatModel.current_variables().continuous_variables(test_pt);
	  fHatModel.evaluate();
	  const Response& gp_resp = fHatModel.current_response();
	  const RealVector& gp_fn = gp_resp.function_values();
	  gp_out << '\n' << std::setw(13) << test_pt[0] << ' ' << std::setw(13)
		 << test_pt[1] << ' ' << std::setw(13) << gp_fn[i];

	  RealVector variances
	    = fHatModel.approximation_variances(fHatModel.current_variables());
	  var_out << '\n' << std::setw(13) << test_pt[0] << ' ' << std::setw(13)
		  << test_pt[1] << ' ' << std::setw(13) << variances[i];

	  if (i==numFunctions-1) {
	    RealVector merit(1);
	    merit[0] = augmented_lagrangian(gp_fn);
	    eif_out << '\n' << std::setw(13) << test_pt[0] << ' '
		    << std::setw(13) << test_pt[1] << ' ' << std::setw(13)
		    << compute_expected_improvement(merit, test_pt);
	  }
	}
	gp_out  << std::endl;
	var_out << std::endl;
	if (i == numFunctions - 1)
	  eif_out << std::endl;
      }
    }
  }
#endif // DEBUG_PLOTS
}


/*
void EffGlobalMinimizer::declare_sources()
{
  // This override exists purely to prevent an optimizer/minimizer from
  // declaring sources when it's being used to evaluate a user-defined
  // function (e.g. finding the correlation lengths of Dakota's GP).

  if (setUpType == "user_functions")
    return;
  else
    Iterator::declare_sources();
}


void EffGlobalMinimizer::derived_init_communicators(ParLevLIter pl_iter)
{
  //approxSubProbModel.init_communicators(pl_iter, maxEvalConcurrency);
    SurrBasedMinimizer::derived_init_communicators(pl_iter);
}


void EffGlobalMinimizer::derived_set_communicators(ParLevLIter pl_iter)
{
  //approxSubProbModel.set_communicators(pl_iter, maxEvalConcurrency);
    SurrBasedMinimizer::derived_set_communicators(pl_iter);
}


void EffGlobalMinimizer::derived_free_communicators(ParLevLIter pl_iter)
{
    SurrBasedMinimizer::derived_free_communicators(pl_iter);
  //approxSubProbModel.free_communicators(pl_iter, maxEvalConcurrency);
}
*/

} // namespace Dakota
