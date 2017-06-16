/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGlobalInterval.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "NonDLHSSampling.hpp"
#include "DakotaModel.hpp"
#include "DakotaIterator.hpp"
#include "RecastModel.hpp"
#include "DataFitSurrModel.hpp"
#include "ProblemDescDB.hpp"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#ifdef HAVE_ACRO
#include "COLINOptimizer.hpp"
#endif
#include "pecos_stat_util.hpp"

//#define DEBUG

namespace Dakota {

// initialization of statics
NonDGlobalInterval* NonDGlobalInterval::nondGIInstance(NULL);


NonDGlobalInterval::NonDGlobalInterval(ProblemDescDB& problem_db, Model& model):
  NonDInterval(problem_db, model),
  seedSpec(probDescDB.get_int("method.random_seed")),
  numSamples(probDescDB.get_int("method.samples")),
  rngName(probDescDB.get_string("method.random_number_generator")),
  allResponsesPerIter(false), dataOrder(1), distanceTol(convergenceTol),
  distanceConvergeLimit(1), improvementConvergeLimit(2)
{
  bool err_flag = false;

  // Define optimization sub-problem solver
  unsigned short opt_alg = probDescDB.get_ushort("method.sub_method");
  bool discrete = (numDiscIntEpistUncVars || numDiscRealEpistUncVars);
  if (opt_alg == SUBMETHOD_EGO) {
    eifFlag = gpModelFlag = true;
    if (discrete) {
      Cerr << "Error: discrete variables are not currently supported for EGO "
	   << "solver in NonDGlobalInterval.  Please select SBO." << std::endl;
      err_flag = true;
    }
  }
  else if (opt_alg == SUBMETHOD_SBO)
    { eifFlag = false; gpModelFlag = true; }
  else if (opt_alg == SUBMETHOD_EA)
    eifFlag = gpModelFlag = false;
  else if (opt_alg == SUBMETHOD_DEFAULT)
    { gpModelFlag = true; eifFlag = (discrete) ? false : true; }
  else {
    Cerr << "Error: unsupported optimization algorithm selection in "
	 << "NonDGlobalInterval.  Please select EGO, SBO, or EA." << std::endl;
    err_flag = true;
  }

  if (numUncertainVars != numContIntervalVars + numDiscIntervalVars +
      numDiscSetIntUncVars + numDiscSetRealUncVars) {
    Cerr << "\nError: only continuous and discrete epistemic variables are "
	 << "currently supported in NonDGlobalInterval." << std::endl;
    err_flag = true;
  }

  if (gpModelFlag) {
    if (!numSamples) // use a default of #terms in a quadratic polynomial
      numSamples = (numUncertainVars+1)*(numUncertainVars+2)/2;
    String approx_type = 
      (probDescDB.get_short("method.nond.emulator") == GP_EMULATOR) ?
      "global_gaussian" : "global_kriging";
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    String sample_reuse = "none";
    if (probDescDB.get_bool("method.derivative_usage")) {
      if (approx_type == "global_gaussian") {
	Cerr << "\nError: efficient_global does not support gaussian_process "
	     << "when derivatives present; use kriging instead." << std::endl;
	err_flag = true;
      }
      if (iteratedModel.gradient_type() != "none") dataOrder |= 2;
      if (iteratedModel.hessian_type()  != "none") dataOrder |= 4;
    }
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_build_points_file");
    if (!import_pts_file.empty())
      { numSamples = 0; sample_reuse = "all"; }
 
    // instantiate the Gaussian Process Model/Iterator recursions

    // The following uses on the fly derived ctor:
    short mode = (eifFlag) ? ACTIVE_UNIFORM : ACTIVE;
    daceIterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
      numSamples, seedSpec, rngName, false, mode), false);
    // only use derivatives if the user requested and they are available
    ActiveSet dace_set = daceIterator.active_set(); // copy
    dace_set.request_values(dataOrder);
    daceIterator.active_set(dace_set);

    // Construct fHatModel using a GP approximation over the active/uncertain
    // vars (same view as iteratedModel: not the typical All view for DACE).
    //
    // Note: low order trend is more robust when there is limited data, such as
    // a few discrete values --> nan's observed for quad trend w/ 2 model forms
    unsigned short trend_order = (discrete) ? 1 : 2;
    UShortArray approx_order(numUncertainVars, trend_order);
    short corr_order = -1, corr_type = NO_CORRECTION;
    //const Variables& curr_vars = iteratedModel.current_variables();
    ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
    gp_set.request_values(1);// no surr deriv evals, but GP may be grad-enhanced
    fHatModel.assign_rep(new DataFitSurrModel(daceIterator, iteratedModel,
      gp_set, approx_type, approx_order, corr_type, corr_order, dataOrder,
      outputLevel, sample_reuse, import_pts_file,
      probDescDB.get_ushort("method.import_build_format"),
      probDescDB.get_bool("method.import_build_active_only"),
      probDescDB.get_string("method.export_approx_points_file"),
      probDescDB.get_ushort("method.export_approx_format")), false);

    // Following this ctor, IteratorScheduler::init_iterator() initializes the
    // parallel configuration for NonDGlobalInterval + iteratedModel using
    // NonDGlobalInterval's maxEvalConcurrency.  During fHatModel construction
    // above, DataFitSurrModel::derived_init_communicators() initializes the
    // parallel configuration for daceIterator + iteratedModel using
    // daceIterator's maxEvalConcurrency.  The only iteratedModel concurrency
    // currently exercised is that used by daceIterator within the initial GP
    // construction, but the NonDGlobalInterval maxEvalConcurrency must still be
    // set so as to avoid parallel config errors resulting from avail_procs
    // > max_concurrency within IteratorScheduler::init_iterator().  Max of the
    // local deriv concurrency & the DACE concurrency is used for this purpose.
    maxEvalConcurrency = std::max(maxEvalConcurrency,
      daceIterator.maximum_evaluation_concurrency());
  }
  else
    fHatModel = iteratedModel; // shared rep

  if (err_flag)
    abort_handler(-1);

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level within the run fn.
  SizetArray recast_vars_comps_total;  // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  short recast_resp_order = 1; // nongradient-based optimizers
  intervalOptModel.assign_rep(
    new RecastModel(fHatModel, recast_vars_comps_total, all_relax_di,
		    all_relax_dr, 1, 0, 0, recast_resp_order), false);

  // Instantiate the optimizer used on the GP.
  // TO DO: add support for discrete EGO
  if (eifFlag) { // EGO solver

    // preserve these EGO settings for now, but eventually map through
    // from spec (and update test baselines)
    convergenceTol = 1.e-12; distanceTol = 1.e-8;
    if (maxIterations < 0) 
      maxIterations  = 25*numContinuousVars;
    
    double min_box_size = 1.e-15, vol_box_size = 1.e-15;
    int max_direct_iter = 1000, max_direct_eval = 10000; // 10*defaults
#ifdef HAVE_NCSU  
    // EGO with DIRECT (exploits GP variance)
    intervalOptimizer.assign_rep(new
      NCSUOptimizer(intervalOptModel, max_direct_iter, max_direct_eval,
		    min_box_size, vol_box_size), false);
#else
    Cerr << "NCSU DIRECT Optimizer is not available to use to find the" 
	 << " interval bounds from the GP model." << std::endl;
    abort_handler(-1);
#endif // HAVE_NCSU
  }
  else { // EAminlp, with or without GP emulation
    int max_ea_iter, max_ea_eval;
    if (gpModelFlag) // SBGO controls from user spec; EA controls hard-wired
      { max_ea_iter = 50; max_ea_eval = 5000; } // default EA pop_size = 100
    else // EA controls from user spec
      { max_ea_iter = maxIterations; max_ea_eval = maxFunctionEvals; }

#ifdef HAVE_ACRO
    // mixed EA (ignores GP variance)
    intervalOptimizer.assign_rep(new
      COLINOptimizer("coliny_ea", intervalOptModel, seedSpec, max_ea_iter,
		     max_ea_eval), false);
//#elif HAVE_JEGA
//    intervalOptimizer.assign_rep(new
//      JEGAOptimizer(intervalOptModel, max_iter, max_eval, min_box_size,
//      vol_box_size), false);
#else
    Cerr << "Error: mixed EA not available for computing interval bounds."
	 << std::endl;
    abort_handler(-1);
#endif // HAVE_NCSU
  }
}


NonDGlobalInterval::~NonDGlobalInterval()
{ }


void NonDGlobalInterval::derived_init_communicators(ParLevLIter pl_iter)
{
  iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // intervalOptModel.init_communicators() recursion is currently sufficient
  // for fHatModel.  An additional fHatModel.init_communicators() call would
  // be motivated by special parallel usage of fHatModel below that is not
  // otherwise covered by the recursion.
  //fHatMaxConcurrency = maxEvalConcurrency; // local derivative concurrency
  //fHatModel.init_communicators(pl_iter, fHatMaxConcurrency);

  // intervalOptimizer uses NoDBBaseConstructor, so no need to manage
  // DB list nodes at this level
  intervalOptimizer.init_communicators(pl_iter);
}


void NonDGlobalInterval::derived_set_communicators(ParLevLIter pl_iter)
{
  NonD::derived_set_communicators(pl_iter);

  //fHatMaxConcurrency = maxEvalConcurrency; // local derivative concurrency
  //fHatModel.set_communicators(pl_iter, fHatMaxConcurrency);

  // intervalOptimizer uses NoDBBaseConstructor, so no need to manage
  // DB list nodes at this level
  intervalOptimizer.set_communicators(pl_iter);
}


void NonDGlobalInterval::derived_free_communicators(ParLevLIter pl_iter)
{
  // intervalOptimizer uses NoDBBaseConstructor, so no need to manage
  // DB list nodes at this level
  intervalOptimizer.free_communicators(pl_iter);

  //fHatMaxConcurrency = maxEvalConcurrency; // local derivative concurrency
  //fHatModel.free_communicators(pl_iter, fHatMaxConcurrency);

  iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}


void NonDGlobalInterval::core_run()
{
  // set the object instance pointer for use within static member functions
  NonDGlobalInterval* prev_instance = nondGIInstance;
  nondGIInstance = this;

  intervalOptModel.update_from_subordinate_model();
  // Build initial GP once for all response functions
  if (gpModelFlag)
    fHatModel.build_approximation();

  Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
  primary_resp_map[0].resize(1);
  BoolDequeArray nonlinear_resp_map(1);
  nonlinear_resp_map[0] = BoolDeque(numFunctions, false);
  BoolDeque max_sense(1);
  RecastModel* int_opt_model_rep = (RecastModel*)intervalOptModel.model_rep();

  initialize(); // virtual fn

  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);

  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {

    primary_resp_map[0][0] = respFnCntr;
    nonlinear_resp_map[0][respFnCntr] = true;
    if (!eifFlag)
      int_opt_model_rep->init_maps(vars_map, false, NULL, NULL,
	primary_resp_map, secondary_resp_map, nonlinear_resp_map, 
	extract_objective, NULL);

    for (cellCntr=0; cellCntr<numCells; ++cellCntr) {

      set_cell_bounds(); // virtual fn for setting bounds for local min/max

      // initialize the recast model for lower bound estimation
      if (eifFlag)
	int_opt_model_rep->init_maps(vars_map, false, NULL, NULL,
	  primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	  EIF_objective_min, NULL);
      else {
	max_sense[0] = false;
	int_opt_model_rep->primary_response_fn_sense(max_sense);
      }

      // Iterate until EGO converges
      distanceConvergeCntr = improvementConvergeCntr = globalIterCntr = 0;
      prevCVStar.size(0); prevDIVStar.size(0); prevDRVStar.size(0);	
      boundConverged = false;
      while (!boundConverged) {
	++globalIterCntr;

	// determine approxFnStar from minimum among sample data
	if (eifFlag)
	  get_best_sample(false, true);

	// Execute GLOBAL search and retrieve results
	Cout << "\n>>>>> Initiating global minimization: response "
	     << respFnCntr+1 << " cell " << cellCntr+1 << " iteration "
	     << globalIterCntr << "\n\n";
	//intervalOptimizer.reset(); // redundant for COLINOptimizer::core_run()
	intervalOptimizer.run(pl_iter);
	// output iteration results, update convergence controls, and update GP
	post_process_run_results(false);
      }
      if (gpModelFlag)
	get_best_sample(false, false); // pull truthFnStar from sample data
      post_process_cell_results(false); // virtual fn: post-process min

      // initialize the recast model for upper bound estimation
      if (eifFlag)
	int_opt_model_rep->init_maps(vars_map, false, NULL, NULL,
	  primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	  EIF_objective_max, NULL);
      else {
	max_sense[0] = true;
	int_opt_model_rep->primary_response_fn_sense(max_sense);
      }

      // Iterate until EGO converges
      distanceConvergeCntr = improvementConvergeCntr = globalIterCntr = 0;
      prevCVStar.size(0); prevDIVStar.size(0); prevDRVStar.size(0);	
      boundConverged = false;
      while (!boundConverged) {
	++globalIterCntr;

	// determine approxFnStar from maximum among sample data
	if (eifFlag)
	  get_best_sample(true, true);

	// Execute GLOBAL search
	Cout << "\n>>>>> Initiating global maximization: response "
	     << respFnCntr+1 << " cell " << cellCntr+1 << " iteration "
	     << globalIterCntr << "\n\n";
	//intervalOptimizer.reset(); // redundant for COLINOptimizer::core_run()
	intervalOptimizer.run(pl_iter);
	// output iteration results, update convergence controls, and update GP
	post_process_run_results(true);
      }
      if (gpModelFlag)
	get_best_sample(true, false); // pull truthFnStar from sample data
      post_process_cell_results(true); // virtual fn: post-process max
    }
    post_process_response_fn_results(); // virtual fn: post-process respFn
    nonlinear_resp_map[0][respFnCntr] = false; // reset
  }
  post_process_final_results(); // virtual fn: final post-processing

  // restore in case of recursion
  nondGIInstance = prev_instance;
}


void NonDGlobalInterval::initialize()
{ } // default is no-op


void NonDGlobalInterval::set_cell_bounds()
{ } // default is no-op


void NonDGlobalInterval::post_process_run_results(bool maximize)
{
  const Variables&     vars_star = intervalOptimizer.variables_results();
  const RealVector&  c_vars_star = vars_star.continuous_variables();
  const IntVector&  di_vars_star = vars_star.discrete_int_variables();
  const RealVector& dr_vars_star = vars_star.discrete_real_variables();
  const Response&      resp_star = intervalOptimizer.response_results();
  Real fn_star = resp_star.function_value(0), fn_conv, dist_conv;

  Cout << "\nResults of interval optimization:\nFinal point             =\n";
  if (vars_star.cv())  write_data(Cout,  c_vars_star);
  if (vars_star.div()) write_data(Cout, di_vars_star);
  if (vars_star.drv()) write_data(Cout, dr_vars_star);
  if (eifFlag)
    Cout << "Expected Improvement    =\n                     "
	 << std::setw(write_precision+7) << -fn_star << '\n';
  else {
    if (gpModelFlag) Cout << "Estimate of ";
    if (maximize)    Cout << "Upper Bound =\n                     ";
    else             Cout << "Lower Bound =\n                     ";
    Cout << std::setw(write_precision+7) << fn_star << '\n';
  }

  if (!gpModelFlag)
    { truthFnStar = fn_star; boundConverged = true; return; }

  if (prevCVStar.empty() && prevDIVStar.empty() && prevDRVStar.empty())
    dist_conv = fn_conv = DBL_MAX; // first iteration
  else if (eifFlag) {
    // Euclidean distance of successive optimal solns: continuous variables only
    dist_conv = rel_change_L2(c_vars_star, prevCVStar);

    // EIF values directly provide estimates of soln convergence
    fn_conv = -fn_star; // EI negated for minimization
    // If DIRECT failed to find a point with EIF>0, it returns the center point
    // as the optimal solution. EGO may have converged, but DIRECT may have just
    // failed to find a point with a good EIF value. Adding this midpoint can
    // alter the GPs enough to allow DIRECT to find something useful, so we
    // force max(EIF)<tol twice to make sure. Note that we cannot make this
    // check more than 2 because it would cause EGO to add the center point
    // more than once, which will damage the GPs.  Unfortunately, when it
    // happens the second time, it may still be that DIRECT failed and not
    // that EGO converged.

    // GT: The only reason we introduce this additional level of convergence is
    // the hope that adding the midpoint will improve the GP enough so that
    // 1. non-monotonic convergence can be addressed by improving the quality of
    // the GP in the hope that if a better soln exists, it can be captured by
    // the new GP or
    // 2. we construct a 'better' GP in the hope that DIRECT will actually find
    // a soln pt.  Furthermore, we do not require this to occur on consecutive
    // runs for the same reasons that we do not add points within the dist_tol.
  }
  else {
    // Euclidean distance of successive optimal solns: continuous,
    // discrete int, and discrete real variables
    dist_conv = rel_change_L2(c_vars_star, prevCVStar, di_vars_star,
			      prevDIVStar, dr_vars_star, prevDRVStar);
    // for SBO, reference fn_star to previous value
    fn_conv = std::abs(1. - fn_star / prevFnStar);// change in lower,upper bound
  }

  // update convergence counters
  if (dist_conv < distanceTol)    ++distanceConvergeCntr;
  if (fn_conv   < convergenceTol) ++improvementConvergeCntr;

  // depending on convergence assessment, we may update the GP, converge
  // the iteration and update the GP, or converge without updating the GP.
  if (globalIterCntr >= maxIterations)
    { boundConverged = true; evaluate_response_star_truth(); }
  else if (distanceConvergeCntr    >= distanceConvergeLimit ||
	   improvementConvergeCntr >= improvementConvergeLimit)
    // if successive iterates are very similar, we do not add the training pt,
    // since the danger of damaging the GP outweighs small possible gains.
    boundConverged = true;
  else { // evaluate truth response and update GP + prev solution trackers
    evaluate_response_star_truth();
    // update previous solution tracking
    if (vars_star.cv())  copy_data( c_vars_star, prevCVStar);
    if (vars_star.div()) copy_data(di_vars_star, prevDIVStar);
    if (vars_star.drv()) copy_data(dr_vars_star, prevDRVStar);
    if (!eifFlag) prevFnStar = fn_star;
  }
}


void NonDGlobalInterval::evaluate_response_star_truth()
{
  //fHatModel.component_parallel_mode(TRUTH_MODEL);
  const Variables& vars_star = intervalOptimizer.variables_results();
  iteratedModel.active_variables(vars_star);
  ActiveSet set = iteratedModel.current_response().active_set();
  // GT: Get all responses per function evaluation
  // changing this might break some of the logic needed to determine
  // whether the inner loop surrogate needs to be reconstructed
  if (allResponsesPerIter)
    set.request_values(dataOrder);
  else
    { set.request_values(0); set.request_value(dataOrder, respFnCntr); }
  iteratedModel.evaluate(set);

  // Update the GP approximation
  IntResponsePair resp_star_truth(iteratedModel.evaluation_id(),
				  iteratedModel.current_response());
  fHatModel.append_approximation(vars_star, resp_star_truth, true);
}


void NonDGlobalInterval::get_best_sample(bool maximize, bool eval_approx)
{ } // default is no-op


void NonDGlobalInterval::post_process_cell_results(bool maximize)
{ } // default is no-op


void NonDGlobalInterval::post_process_response_fn_results()
{ } // default is no-op


void NonDGlobalInterval::post_process_final_results()
{ } // default is no-op


void NonDGlobalInterval::
extract_objective(const Variables& sub_model_vars, const Variables& recast_vars,
		  const Response& sub_model_response, Response& recast_response)
{
  // minimize or maximize sense is set separately, so this fn only
  // extracts the active response fn using respFnCntr

  Real sub_model_fn
    = sub_model_response.function_values()[nondGIInstance->respFnCntr];
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1)
    recast_response.function_value(sub_model_fn, 0);
  // Note: could track c/di/drVarsStar and approxFnStar here
}


void NonDGlobalInterval::
EIF_objective_min(const Variables& sub_model_vars, const Variables& recast_vars,
		  const Response& sub_model_response, Response& recast_response)
{
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = nondGIInstance->fHatModel.approximation_variances(recast_vars);

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) {
    // max(EI) for both interval bounds --> return -EI to the minimizer
    const Real& mean = means[nondGIInstance->respFnCntr];
    Real stdv = sqrt(variances[nondGIInstance->respFnCntr]);
    const Real& approx_fn_star = nondGIInstance->approxFnStar;
    // Calculate expected improvement: +/-approx_fn_star used for EIF_min/max
    Real Phi_snv, phi_snv, snv = approx_fn_star - mean;
    if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
      phi_snv = 0.;
      Phi_snv = (snv > 0.) ? 1. : 0.;
    }
    else{
      snv /= stdv; // now snv is the standard normal variate
      Phi_snv = Pecos::NormalRandomVariable::std_cdf(snv); 
      phi_snv = Pecos::NormalRandomVariable::std_pdf(snv);
    }
    Real ei = (approx_fn_star - mean)*Phi_snv + stdv*phi_snv;
    // both bounds maximize EIF -> minimize -EIF
    recast_response.function_value(-ei, 0);
#ifdef DEBUG
    Cout << "(Evaluation,ApproxFnStar,Phi,phi,vars): (" << mean << "," 
	 << approx_fn_star << "," << Phi_snv << "," << phi_snv;
    const RealVector& eval_vars = recast_vars.continuous_variables();
    for (size_t i=0; i < eval_vars.length(); i++)
      Cout << ", " << eval_vars[i];
    Cout << ")\n";
#endif
  }
}


void NonDGlobalInterval::
EIF_objective_max(const Variables& sub_model_vars, const Variables& recast_vars,
		  const Response& sub_model_response, Response& recast_response)
{
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = nondGIInstance->fHatModel.approximation_variances(recast_vars);

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) {
    // max(EI) for both interval bounds --> return -EI to the minimizer
    Real mean = -means[nondGIInstance->respFnCntr],
         stdv = sqrt(variances[nondGIInstance->respFnCntr]);
    const Real& approx_fn_star = nondGIInstance->approxFnStar;
    // Calculate expected improvement: +/-approx_fn_star used for EIF_min/max
    Real Phi_snv, phi_snv, snv = -approx_fn_star - mean;
    if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
      phi_snv = 0.;
      Phi_snv = (snv > 0.) ? 1. : 0.;
    }
    else{
      snv /= stdv; // now snv is the standard normal variate   
      Phi_snv = Pecos::NormalRandomVariable::std_cdf(snv);
      phi_snv = Pecos::NormalRandomVariable::std_pdf(snv);
    }
    Real ei = (-approx_fn_star - mean)*Phi_snv + stdv*phi_snv;
    // both bounds maximize EIF -> minimize -EIF
    recast_response.function_value(-ei, 0);
#ifdef DEBUG
    Cout << "(Evaluation,ApproxFnStar,Phi,phi,vars): (" << mean << "," 
	 << approx_fn_star << "," << Phi_snv << "," << phi_snv;
    const RealVector& eval_vars = recast_vars.continuous_variables();
    for (size_t i=0; i < eval_vars.length(); i++)
      Cout << ", " << eval_vars[i];
    Cout << ")\n";	
#endif	
  }
}

} // namespace Dakota
