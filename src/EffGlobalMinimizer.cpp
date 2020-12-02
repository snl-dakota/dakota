/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EffGlobalMinimizer
//- Description: Implementation code for the EffGlobalMinimizer class
//- Owner:       Barron J Bichon, Vanderbilt University
//- Checked by:
//- Version:

//- Edited by:   Anh Tran in 2020 for parallelization

#include "EffGlobalMinimizer.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "NonDLHSSampling.hpp"
#include "RecastModel.hpp"
#include "DataFitSurrModel.hpp"
#include "DakotaApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaGraphics.hpp"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NormalRandomVariable.hpp"
#include <boost/lexical_cast.hpp>

//#define DEBUG
//#define DEBUG_PLOTS

namespace Dakota {

EffGlobalMinimizer* EffGlobalMinimizer::effGlobalInstance(NULL);


// This constructor accepts a Model
EffGlobalMinimizer::EffGlobalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedMinimizer(problem_db, model,
		     std::shared_ptr<TraitsBase>(new EffGlobalTraits())),
  batchSize(probDescDB.get_int("method.batch_size")),
  batchSizeExploration(probDescDB.get_int("method.batch_size.exploration")),
  //setUpType("model"),
  dataOrder(1), batchAsynch(false)
{
  // substract the total batchSize from batchSizeExploration
  batchSizeAcquisition = batchSize - batchSizeExploration;

  // historical default convergence tolerances
  if (convergenceTol < 0.) convergenceTol = 1.e-12;
  distanceTol = probDescDB.get_real("method.x_conv_tol");
  if (distanceTol < 0.) distanceTol = 1.e-8;

  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

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

  truthFnStar.resize(numFunctions);

  // Always build a global Gaussian process model.  No correction is needed.
  String approx_type;
  switch (probDescDB.get_short("method.nond.emulator")) {
  case GP_EMULATOR:     approx_type = "global_gaussian";
  case EXPGP_EMULATOR:  approx_type = "global_exp_gauss_proc";
  default:              approx_type = "global_kriging";
  }

  String sample_reuse = "none"; // *** TO DO: allow reuse separate from import
  UShortArray approx_order; // empty
  short corr_order = -1, corr_type = NO_CORRECTION;
  if (probDescDB.get_bool("method.derivative_usage")) {
    if (approx_type == "global_gaussian") {
      Cerr << "\nError: efficient_global does not support gaussian_process "
	   << "when derivatives present; use kriging instead." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    if (iteratedModel.gradient_type() != "none") dataOrder |= 2;
    if (iteratedModel.hessian_type()  != "none") dataOrder |= 4;
  }
  int db_samples = probDescDB.get_int("method.samples");
  int samples = (db_samples > 0) ? db_samples :
    (numContinuousVars+1)*(numContinuousVars+2)/2;
  int lhs_seed = probDescDB.get_int("method.random_seed");
  unsigned short sample_type = SUBMETHOD_DEFAULT;
  String rng; // empty string: use default
  //int symbols = samples; // symbols needed for DDACE
  bool vary_pattern = false;// for consistency across any outer loop invocations
  // get point samples file
  const String& import_pts_file
    = probDescDB.get_string("method.import_build_points_file");
  if (!import_pts_file.empty()) // *** TO DO: allow reuse separate from import
    { samples = 0; sample_reuse = "all"; }

  Iterator dace_iterator;
  // The following uses on the fly derived ctor:
  dace_iterator.assign_rep(std::make_shared<NonDLHSSampling>(iteratedModel,
    sample_type, samples, lhs_seed, rng, vary_pattern, ACTIVE_UNIFORM));
  // only use derivatives if the user requested and they are available
  dace_iterator.active_set_request_values(dataOrder);

  // Construct f-hat (fHatModel) using a GP approximation for each response
  // function over the active/design vars (same view as iteratedModel:
  // not the typical All view for DACE).
  //const Variables& curr_vars = iteratedModel.current_variables();
  ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
  gp_set.request_values(1); // no surr deriv evals, but GP may be grad-enhanced
  fHatModel.assign_rep(std::make_shared<DataFitSurrModel>(dace_iterator,
    iteratedModel, gp_set, approx_type, approx_order, corr_type, corr_order,
    dataOrder, outputLevel, sample_reuse, import_pts_file,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")));
  
  // Following this ctor, IteratorScheduler::init_iterator() initializes the
  // parallel configuration for EffGlobalMinimizer + iteratedModel using
  // EffGlobalMinimizer's maxEvalConcurrency.  During fHatModel construction
  // above, DataFitSurrModel::derived_init_communicators() initializes the
  // parallel config for dace_iterator + iteratedModel using dace_iterator's
  // maxEvalConcurrency.  The only iteratedModel concurrency currently exercised
  // is that used by dace_iterator within the initial GP construction, but the
  // EffGlobalMinimizer maxEvalConcurrency must still be set so as to avoid
  // parallel config errors resulting from avail_procs > max_concurrency within
  // IteratorScheduler::init_iterator().  A max of the local derivative
  // concurrency and the DACE concurrency is used for this purpose.
  maxEvalConcurrency = std::max(maxEvalConcurrency,
				dace_iterator.maximum_evaluation_concurrency());

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level at run time.
  SizetArray recast_vars_comps_total; // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  short recast_resp_order = 1; // nongradient-based optimizers
  approxSubProbModel.assign_rep(std::make_shared<RecastModel>(fHatModel,
    recast_vars_comps_total, all_relax_di, all_relax_dr, 1, 0, 0,
    recast_resp_order));

  // must use alternate NoDB ctor chain
  int max_iterations = 10000, max_fn_evals = 50000;
  double min_box_size = 1.e-15, vol_box_size = 1.e-15;
#ifdef HAVE_NCSU
  approxSubProbMinimizer.assign_rep(std::make_shared<NCSUOptimizer>(
    approxSubProbModel, max_iterations, max_fn_evals,
    min_box_size, vol_box_size));
#else
  Cerr << "NCSU DIRECT is not available to optimize the GP subproblems. "
       << "Aborting process." << std::endl;
  abort_handler(METHOD_ERROR);
#endif //HAVE_NCSU
}


void EffGlobalMinimizer::pre_run()
{
  //Minimizer::pre_run(); // invoke any base class definition (none defined)

  if (!approxSubProbModel.mapping_initialized()) {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    /*bool var_size_changed =*/ approxSubProbModel.initialize_mapping(pl_iter);
    //if (var_size_changed) resize();
  }

  // initialize convergence and flag variables
  initialize_counters_limits();
  // check if iterated model supports asynchronous parallelism
  check_parallelism();
  // initialize persistent batch arrays
  varsArrayBatch.resize(batchSize); // size for acquisition + exploration
}


void EffGlobalMinimizer::core_run()
{
  EffGlobalMinimizer* prev_instance = effGlobalInstance;
  effGlobalInstance = this; // instance pointer used within static member fns

  //if (setUpType=="model") {

    // build initial GP for all response functions
    build_gp(); // TO DO: consider moving to pre_run() to enable alt workflow

    // iteratively adapt the GP (in parallel) until EGO converges
    if (batchAsynch && parallelFlag) batch_asynchronous_ego();
    else                             batch_synchronous_ego();

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
  varsArrayBatch.clear(); // clear memory

  if (approxSubProbModel.mapping_initialized())
    approxSubProbModel.finalize_mapping();

  Minimizer::post_run(s);
}


void EffGlobalMinimizer::check_parallelism()
{
  // Add safeguard: If model does not support asynchronous evals, then reset
  // batch sizes for serial execution and echo a warning

  parallelFlag = false; // default: run sequential by default
  if (batchSizeAcquisition > 1 || batchSizeExploration > 1) {
    if (iteratedModel.asynch_flag())
      parallelFlag = true; // turn on if requirements are satisfied
    else {
      Cerr << "Warning: concurrent operations not supported by model. "
	   << "Batch size request ignored." << std::endl;
      batchSize = batchSizeAcquisition = 1; // revert to sequential default
      batchSizeExploration = 0; // revert
    }
  }
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
  approxSubProbModel.primary_response_fn_weights(RealVector(), false); // no recursion
  approxSubProbModel.reshape_constraints(0, 0,
    approxSubProbModel.num_linear_ineq_constraints(),
    approxSubProbModel.num_linear_eq_constraints());

  // Build initial GP once for all response functions
  fHatModel.build_approximation();
}


void EffGlobalMinimizer::batch_synchronous_ego()
{
  bool approx_converged = false;
  while (!approx_converged) {

    // Need to be less aggressive about convergence when you are accumulating
    // liar evaluations...
    // *** TO DO: consider avoiding these increments unless truth eval...
    // *** handle in a similar way to nonlinear constraint multipliers/penalties
    if (parallelFlag) {
      // reset the convergence counters
      distConvergenceCntr = 0; // reset distance convergence counters
      //distConvergenceLimit
      //  = std::max(batchSizeAcquisition, batchSizeExploration);
      distConvergenceLimit = batchSize; // reset conv limit for parallel EGO
    }

    // construct the acquisition batch
    if (batchSizeAcquisition) construct_batch_acquisition();
    // construct the exploration batch
    if (batchSizeExploration) construct_batch_exploration();

    // blocking synch for composite batch (acquisition + exploration)
    evaluate_batch();

    // check convergence
    approx_converged = assess_convergence();
  }
}


void EffGlobalMinimizer::batch_asynchronous_ego()
{
  bool approx_converged = false;
  while (!approx_converged) {

    // parallelFlag is true: reset the convergence counters
    // *** TO DO: see above
    distConvergenceCntr  = 0; // reset distance convergence counters
    distConvergenceLimit = batchSize; // reset conv limit for parallel EGO

    size_t running_acq  = 0,// acqBatchQueue.size(),
           running_expl = 0,//explBatchQueue.size(),
           new_acq  = batchSizeAcquisition - running_acq,
           new_expl = batchSizeExploration - running_expl;

    // construct the acquisition batch
    if (new_acq)  construct_batch_acquisition();
    // construct the exploration batch
    if (new_expl) construct_batch_exploration();

    // non-blocking synch for composite batch (acquisition + exploration)
    //query_batch(); // *** TO DO

    // check convergence
    approx_converged = assess_convergence();
  }
}


void EffGlobalMinimizer::construct_batch_acquisition()
{
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
  bool append_liars = (batchSize > 1);
  int start_eval_id = iteratedModel.evaluation_id() + 1;
  for (i=0; i<batchSizeAcquisition; ++i) {

    Cout << "\n>>>>> Initiating global iteration " << ++globalIterCount
	 << " (acquisition batch " << i+1 << ")\n";
    
    // determine fnStar from among sample data
    get_best_sample();

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

    update_convergence_counters(vars_star, ei_resp_star);
    if (append_liars)
      append_liar(vars_star, start_eval_id + i);

    // save a copy for truth replacement downstream
    varsArrayBatch[i] = vars_star.copy();
  }
}


void EffGlobalMinimizer::construct_batch_exploration()
{
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
  bool append_liars = (batchSize > 1);
  int start_eval_id = iteratedModel.evaluation_id() + batchSizeAcquisition + 1;
  for (i=0; i<batchSizeExploration; ++i) {

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

    update_convergence_counters(vars_star);//, pv_resp_star); // *** TO DO
    if (append_liars)
      append_liar(vars_star, start_eval_id + i);

    // save a copy for truth replacement downstream
    varsArrayBatch[batchSizeAcquisition + i] = vars_star.copy();
  }
}


void EffGlobalMinimizer::append_liar(const Variables& vars_star, int liar_id)
{
  // get approximate (liar) response value for optimal point (vars_star)
  fHatModel.active_variables(vars_star);
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
  fHatModel.append_approximation(vars_star, liar_resp_pr, true);
  //numDataPts = fHatModel.approximation_data(0).points(); // updated count
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Parallel EGO: liar response appended.\n";
}


void EffGlobalMinimizer::evaluate_batch()
{
  if (parallelFlag) {

    // remove all liar responses prior to replacement with truth
    pop_liar_responses();

    // queue evaluations for composite batch (acquisition + exploration)
    for (int i=0; i<batchSize; ++i) {
      fHatModel.component_parallel_mode(TRUTH_MODEL_MODE);
      iteratedModel.active_variables(varsArrayBatch[i]);
      ActiveSet set = iteratedModel.current_response().active_set();
      set.request_values(dataOrder);
      iteratedModel.evaluate_nowait(set);
    }

    // blocking synchronize: evaluate true responses in parallel
    const IntResponseMap& truth_resp_map = iteratedModel.synchronize();

    // update the GP approximation with batch results
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "\nParallel EGO: adding true responses...\n";
    fHatModel.append_approximation(varsArrayBatch, truth_resp_map, true);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "\nParallel EGO: all true responses added.\n";

    // update constraints (truth resp only, not for liar resp)
    if (numNonlinearConstraints)
      for (IntRespMCIter r_cit = truth_resp_map.begin();
	   r_cit != truth_resp_map.end(); ++r_cit)
	update_constraints(r_cit->second.function_values());
  }
  else {
    // no pop: liar was not appended in serial case

    // serial evaluation
    fHatModel.component_parallel_mode(TRUTH_MODEL_MODE);
    const Variables& vars_star = varsArrayBatch[0];
    iteratedModel.active_variables(vars_star);
    ActiveSet set = iteratedModel.current_response().active_set();
    set.request_values(dataOrder);
    iteratedModel.evaluate(set);

    // update the GP approximation
    const Response& truth_resp = iteratedModel.current_response();
    IntResponsePair truth_resp_pr(iteratedModel.evaluation_id(), truth_resp);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "\nParallel EGO: adding true response...\n";
    fHatModel.append_approximation(vars_star, truth_resp_pr, true);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "\nParallel EGO: true response added.\n";

    // update constraints (truth resp only, not for liar resp)
    if (numNonlinearConstraints)
      update_constraints(truth_resp.function_values());
  }
}


bool EffGlobalMinimizer::assess_convergence()
{ 
  // set convergence flag if any counters have reached their limits
  bool converged = ( distConvergenceCntr >= distConvergenceLimit ||
		     eifConvergenceCntr  >= eifConvergenceLimit ||
		     globalIterCount     >= maxIterations );

  if (outputLevel > NORMAL_OUTPUT) { // if verbose or debug
    if (distConvergenceCntr >= distConvergenceLimit)
      Cout << "\nStopping criteria met: distConvergenceCntr (="
	   << distConvergenceCntr << ") >= ";
    else
      Cout << "\nStopping criteria not met: distConvergenceCntr (="
	   << distConvergenceCntr << ") < ";
    Cout << "distConvergenceLimit (=" << distConvergenceLimit << ").\n";

    if (eifConvergenceCntr >= eifConvergenceLimit)
      Cout << "\nStopping criteria met: eifConvergenceCntr (="
	   << eifConvergenceCntr << ") >= ";
    else
      Cout << "\nStopping criteria not met: eifConvergenceCntr (="
	   << eifConvergenceCntr << ") < ";
    Cout << "eifConvergenceLimit (=" << eifConvergenceLimit << ").\n";

    if (globalIterCount >= maxIterations)
      Cout << "\nStopping criteria met: globalIterCount (="
	   << globalIterCount << ") >= ";
    else
      Cout << "\nStopping criteria not met: globalIterCount (="
	   << globalIterCount << ") < ";
    Cout << "maxIterations (=" << maxIterations << ").\n";
  }

  return converged;
}


void EffGlobalMinimizer::
update_convergence_counters(const Variables& vars_star)
{
  // check for convergence based in distance between successive points
  // If the dist between successive points is very small, then there is
  // little value in updating the GP since the new training point will
  // essentially be the previous optimal point.
  const RealVector& c_vars = vars_star.continuous_variables();
  distCStar = (prevCvStar.empty()) ? DBL_MAX
                                   : rel_change_L2(c_vars, prevCvStar);
  if (distCStar < distanceTol)
    ++distConvergenceCntr;

  // update prevCvStar
  copy_data(c_vars, prevCvStar); // *** TO DO: distinguish truth vs. liar?

  if (outputLevel >= DEBUG_OUTPUT) debug_print_values(vars_star);
}


void EffGlobalMinimizer::
update_convergence_counters(const Response& resp_star)
{
  Real eif_star = -resp_star.function_value(0);
  // Check for convergence based on max EIF
  if ( eif_star < convergenceTol ) // Note: based only on approx subprob solve
    ++eifConvergenceCntr;          // *** TO DO: distinguish truth vs. liar?

  // If DIRECT failed to find a point with EIF>0, it returns the center point
  // as the optimal solution. EGO may have converged, but DIRECT may have just
  // failed to find a point with a good EIF value.  Adding this midpoint can
  // alter the GPs enough to allow DIRECT to find something useful, so we force
  // max(EIF)<tol twice to make sure. Note that we cannot make this check more
  // than 2 because it would cause EGO to add the center point more than once,
  // which will damage the GPs.  Unfortunately, when it happens the second time,
  // it may still be that DIRECT failed and not that EGO converged.
  if (outputLevel >= DEBUG_OUTPUT) debug_print_counters(eif_star);
}


void EffGlobalMinimizer::retrieve_final_results()
{
  // Set best variables and response for use by strategy level.
  // c_vars, fmin contain the optimal design
  get_best_sample(); // pull optimal result from sample data
  bestVariablesArray.front().continuous_variables(varStar);
  bestResponseArray.front().function_values(truthFnStar);

  // (conditionally) export final surrogates
  export_final_surrogates(fHatModel);

  debug_plots(); // *** TO DO: perhaps moot, but plot final only?
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


/** Get the best-so-far sample **/
void EffGlobalMinimizer::get_best_sample()
{
  // pull the samples and responses from data used to build latest GP
  // to determine fnStar for use in the expected improvement function

  const Pecos::SurrogateData& gp_data_0 = fHatModel.approximation_data(0);
  const Pecos::SDVArray& sdv_array = gp_data_0.variables_data();
  const Pecos::SDRArray& sdr_array = gp_data_0.response_data();

  size_t i, sam_star_idx = 0, num_data_pts = gp_data_0.points();
  Real fn, fn_star = DBL_MAX;

  for (i=0; i<num_data_pts; ++i) {
    const RealVector& sams = sdv_array[i].continuous_variables();

    fHatModel.continuous_variables(sams);
    fHatModel.evaluate();
    const RealVector& f_hat = fHatModel.current_response().function_values();
    fn = augmented_lagrangian(f_hat);

    if (fn < fn_star) {
      copy_data(sams, varStar);
      sam_star_idx = i;
      fn_star = meritFnStar = fn;
      truthFnStar[0] = sdr_array[i].response_function();
    }
  }

  // update truthFnStar with all additional primary/secondary fns corresponding
  // to lowest merit function value
  for (i=1; i<numFunctions; ++i) {
    const Pecos::SDRArray& sdr_array
      = fHatModel.approximation_data(i).response_data();
    truthFnStar[i] = sdr_array[sam_star_idx].response_function();
  }
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
  for (size_t i=0; i<numFunctions; i++) {
    std::string samsfile("ego_sams");
    std::string tag = "_" + boost::lexical_cast<std::string>(i+1) + ".out";
    samsfile += tag;
    std::ofstream samsOut(samsfile.c_str(),std::ios::out);
    samsOut << std::scientific;
    const Pecos::SurrogateData& gp_data = fHatModel.approximation_data(i);
    const Pecos::SDVArray& sdv_array = gp_data.variables_data();
    const Pecos::SDRArray& sdr_array = gp_data.response_data();
    size_t num_data_pts = gp_data.size(), num_vars = fHatModel.cv();
    for (size_t j=0; j<num_data_pts; ++j) {
      samsOut << '\n';
      const RealVector& sams = sdv_array[j].continuous_variables();
      for (size_t k=0; k<num_vars; k++)
	samsOut << std::setw(13) << sams[k] << ' ';
      samsOut << std::setw(13) << sdr_array[j].response_function();
    }
    samsOut << std::endl;

    // Plotting the GP over a grid is intended for visualization and
    // is therefore only available for 2D problems
    if (num_vars==2) {
      std::string gpfile("ego_gp");
      std::string varfile("ego_var");
      gpfile  += tag;
      varfile += tag;
      std::ofstream  gpOut(gpfile.c_str(),  std::ios::out);
      std::ofstream varOut(varfile.c_str(), std::ios::out);
      std::ofstream eifOut("ego_eif.out",   std::ios::out);
      gpOut  << std::scientific;
      varOut << std::scientific;
      eifOut << std::scientific;
      RealVector test_pt(2);
      const RealVector& lbnd = fHatModel.continuous_lower_bounds();
      const RealVector& ubnd = fHatModel.continuous_upper_bounds();
      Real interval0 = (ubnd[0] - lbnd[0])/100.,
	   interval1 = (ubnd[1] - lbnd[1])/100.;
      for (size_t j=0; j<101; j++) {
	test_pt[0] = lbnd[0] + float(j) * interval0;
	for (size_t k=0; k<101; k++) {
	  test_pt[1] = lbnd[1] + float(k) * interval1;

	  fHatModel.continuous_variables(test_pt);
	  fHatModel.evaluate();
	  const Response& gp_resp = fHatModel.current_response();
	  const RealVector& gp_fn = gp_resp.function_values();

	  gpOut << '\n' << std::setw(13) << test_pt[0] << ' ' << std::setw(13)
		<< test_pt[1] << ' ' << std::setw(13) << gp_fn[i];

	  RealVector variances
	    = fHatModel.approximation_variances(fHatModel.current_variables());

	  varOut << '\n' << std::setw(13) << test_pt[0] << ' ' << std::setw(13)
		 << test_pt[1] << ' ' << std::setw(13) << variances[i];

	  if (i==numFunctions-1) {
	    RealVector merit(1);
	    merit[0] = augmented_lagrangian(gp_fn);

	    Real ei = compute_expected_improvement(merit, test_pt);

	    eifOut << '\n' << std::setw(13) << test_pt[0] << ' '
		   << std::setw(13) << test_pt[1] << ' ' << std::setw(13) << ei;
	  }
	}
	gpOut  << std::endl;
	varOut << std::endl;
	if (i == numFunctions - 1)
	  eifOut << std::endl;
      }
    }
  }
#endif //DEBUG_PLOTS
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
