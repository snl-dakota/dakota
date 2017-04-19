/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EffGlobalMinimizer
//- Description: Implementation code for the EffGlobalMinimizer class
//- Owner:       Barron J Bichon, Vanderbilt University

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
#include "pecos_stat_util.hpp"
#include <boost/lexical_cast.hpp>

//#define DEBUG
//#define DEBUG_PLOTS

namespace Dakota {

EffGlobalMinimizer* EffGlobalMinimizer::effGlobalInstance(NULL);


// This constructor accepts a Model
EffGlobalMinimizer::
EffGlobalMinimizer(ProblemDescDB& problem_db, Model& model): 
  SurrBasedMinimizer(problem_db, model), setUpType("model"), dataOrder(1)
{
  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  // initialize augmented Lagrange multipliers
  size_t num_multipliers = numNonlinearEqConstraints;
  for (size_t i=0; i<numNonlinearIneqConstraints; i++) {
    if (origNonlinIneqLowerBnds[i] > -bigRealBoundSize) // g has a lower bound
      ++num_multipliers;
    if (origNonlinIneqUpperBnds[i] <  bigRealBoundSize) // g has an upper bound
      ++num_multipliers;
  }
  augLagrangeMult.resize(num_multipliers);
  augLagrangeMult = 0.;

  truthFnStar.resize(numFunctions);

  // Always build a global Gaussian process model.  No correction is needed.
  String approx_type = "global_kriging";
  if (probDescDB.get_short("method.nond.emulator") == GP_EMULATOR)
    approx_type = "global_gaussian";

  String sample_reuse = "none";
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
  if (!import_pts_file.empty())
    { samples = 0; sample_reuse = "all"; }

  Iterator dace_iterator;
  // The following uses on the fly derived ctor:
  dace_iterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
    samples, lhs_seed, rng, vary_pattern, ACTIVE_UNIFORM), false);
  // only use derivatives if the user requested and they are available
  ActiveSet dace_set = dace_iterator.active_set(); // copy
  dace_set.request_values(dataOrder);
  dace_iterator.active_set(dace_set);

  // Construct f-hat using a GP approximation for each response function over
  // the active/design vars (same view as iteratedModel: not the typical All
  // view for DACE).
  //const Variables& curr_vars = iteratedModel.current_variables();
  ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
  gp_set.request_values(1); // no surr deriv evals, but GP may be grad-enhanced
  fHatModel.assign_rep(new DataFitSurrModel(dace_iterator, iteratedModel,
    gp_set, approx_type, approx_order, corr_type, corr_order, dataOrder,
    outputLevel, sample_reuse, import_pts_file,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")), false);

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
  // each level within the run fn.
  SizetArray recast_vars_comps_total; // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  short recast_resp_order = 1; // nongradient-based optimizers
  eifModel.assign_rep(
    new RecastModel(fHatModel, recast_vars_comps_total, all_relax_di,
		    all_relax_dr, 1, 0, 0, recast_resp_order), false);

  // must use alternate NoDB ctor chain
  int max_iterations = 10000, max_fn_evals = 50000;
  double min_box_size = 1.e-15, vol_box_size = 1.e-15;
#ifdef HAVE_NCSU
  approxSubProbMinimizer.assign_rep(new NCSUOptimizer(eifModel, max_iterations, 
    max_fn_evals, min_box_size, vol_box_size), false);
#else
  Cerr << "NCSU DIRECT is not available to optimize the GP subproblems. " 
       << "Aborting process." << std::endl;
  abort_handler(METHOD_ERROR);
#endif //HAVE_NCSU
}


/* This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB.
EffGlobalMinimizer::
EffGlobalMinimizer(Model& model, int max_iter, int max_eval) :
  SurrBasedMinimizer(EFFICIENT_GLOBAL, model), setUpType("model")
{ maxIterations = max_iter; maxFunctionEvals = max_eval; }
*/


EffGlobalMinimizer::~EffGlobalMinimizer() 
{ }


/*
// SurrBasedMinimizer implementation of derived_{init,set,free} is sufficient

void EffGlobalMinimizer::derived_init_communicators(ParLevLIter pl_iter)
{
  // iteratedModel is evaluated to add truth data (single evaluate())
  iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // approxSubProbMinimizer.init_communicators() recursion is currently
  // sufficient for fHatModel.  An additional fHatModel.init_communicators()
  // call would be motivated by special parallel usage of fHatModel below that
  // is not otherwise covered by the recursion.
  //fHatMaxConcurrency = maxEvalConcurrency; // local derivative concurrency
  //fHatModel.init_communicators(pl_iter, fHatMaxConcurrency);

  // approxSubProbMinimizer uses NoDBBaseConstructor, so no need to
  // manage DB list nodes at this level
  approxSubProbMinimizer.init_communicators(pl_iter);
}


void EffGlobalMinimizer::derived_free_communicators(ParLevLIter pl_iter)
{
  // deallocate communicators for DIRECT on eifModel
  approxSubProbMinimizer.free_communicators(pl_iter);

  // approxSubProbMinimizer.free_communicators() recursion is currently
  // sufficient for fHatModel.  An additional fHatModel.free_communicators()
  // call would be motivated by special parallel usage of fHatModel below that
  // is not otherwise covered by the recursion.
  //fHatModel.free_communicators(pl_iter, fHatMaxConcurrency);

  // iteratedModel is evaluated to add truth data (single evaluate())
  iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}
*/


void EffGlobalMinimizer::core_run()
{
  if (setUpType=="model")
    minimize_surrogates_on_model();
  else if (setUpType=="user_functions") {
    //minimize_surrogates_on_user_functions();
    Cerr << "Error: bad setUpType in EffGlobalMinimizer::core_run()."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  else {
    Cerr << "Error: bad setUpType in EffGlobalMinimizer::core_run()."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void EffGlobalMinimizer::minimize_surrogates_on_model()
{
  //------------------------------------------------------------------
  //     Solve the problem.
  //------------------------------------------------------------------

  // set the object instance pointers for use within the static member fns
  EffGlobalMinimizer* prev_instance = effGlobalInstance;
  effGlobalInstance = this;

  // now that variables/labels/bounds/targets have flowed down at run-time from
  // any higher level recursions, propagate them up the instantiate-on-the-fly
  // Model recursion so that they are correct when they propagate back down.
  eifModel.update_from_subordinate_model(); // depth = max

  // Build initial GP once for all response functions
  fHatModel.build_approximation();

  // Iterate until EGO converges
  unsigned short eif_convergence_cntr = 0, dist_convergence_cntr = 0,
    eif_convergence_limit = 2, dist_convergence_limit = 1;
  globalIterCount = 0;
  bool approx_converged = false;
  convergenceTol = 1.e-12; Real dist_tol = 1.e-8;
  // Decided for now (10-25-2013) to have EGO take the maxIterations 
  // as the default from minimizer, so it will be initialized as 100
  //  maxIterations  = 25*numContinuousVars;
  RealVector prev_cv_star;
  while (!approx_converged) {
    ++globalIterCount;

    // initialize EIF recast model
    Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
    BoolDequeArray nonlinear_resp_map(1, BoolDeque(numFunctions, true));
    primary_resp_map[0].resize(numFunctions);
    for (size_t i=0; i<numFunctions; i++)
      primary_resp_map[0][i] = i;
    RecastModel* eif_model_rep = (RecastModel*)eifModel.model_rep();
    eif_model_rep->init_maps(vars_map, false, NULL, NULL,
      primary_resp_map, secondary_resp_map, nonlinear_resp_map,
      EIF_objective_eval, NULL);

    // determine fnStar from among sample data
    get_best_sample();

    // Execute GLOBAL search and retrieve results
    Cout << "\n>>>>> Initiating global optimization\n";
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    approxSubProbMinimizer.run(pl_iter);
    const Variables&  vars_star = approxSubProbMinimizer.variables_results();
    const RealVector& c_vars    = vars_star.continuous_variables();
    const Response&   resp_star = approxSubProbMinimizer.response_results();
    const Real&       eif_star  = resp_star.function_value(0);

    // Get expected value for output
    fHatModel.continuous_variables(c_vars);
    fHatModel.evaluate();
    const RealVector& mean = fHatModel.current_response().function_values();
    Real aug_lag = augmented_lagrangian_merit(mean,
      iteratedModel.primary_response_fn_sense(),
      iteratedModel.primary_response_fn_weights(), origNonlinIneqLowerBnds,
      origNonlinIneqUpperBnds, origNonlinEqTargets);

    Cout << "\nResults of EGO iteration:\nFinal point =\n";
    write_data(Cout, c_vars);
    Cout << "Expected Improvement    =\n                     "
	 << std::setw(write_precision+7) << -eif_star
	 << "\n                     " << std::setw(write_precision+7)
	 << aug_lag << " [merit]\n";

#ifdef DEBUG
    RealVector variance = fHatModel.approximation_variances(vars_star);
    RealVector ev = expected_violation(mean,variance);
    RealVector stdv(numFunctions);
    for (size_t i=0; i<numFunctions; i++)
      stdv[i] = std::sqrt(variance[i]);
    Cout << "\nexpected values    =\n" << mean
	 << "\nstandard deviation =\n" << stdv
	 << "\nexpected violation =\n" << ev << std::endl;
#endif //DEBUG

    // Check for convergence based on max EIF
    if ( -eif_star < convergenceTol )
      ++eif_convergence_cntr;

    // Check for convergence based in distance between successive points.
    // If the dist between successive points is very small, then there is
    // little value in updating the GP since the new training point will
    // essentially be the previous optimal point.

    Real dist_cstar = (prev_cv_star.empty()) ? DBL_MAX :
      rel_change_L2(c_vars, prev_cv_star);
    // update prev_cv_star
    copy_data(c_vars, prev_cv_star);
    if (dist_cstar < dist_tol)
      ++dist_convergence_cntr;

    // If DIRECT failed to find a point with EIF>0, it returns the
    //   center point as the optimal solution. EGO may have converged,
    //   but DIRECT may have just failed to find a point with a good
    //   EIF value. Adding this midpoint can alter the GPs enough to
    //   to allow DIRECT to find something useful, so we force 
    //   max(EIF)<tol twice to make sure. Note that we cannot make
    //   this check more than 2 because it would cause EGO to add
    //   the center point more than once, which will damage the GPs.
    //   Unfortunately, when it happens the second time, it may still
    //   be that DIRECT failed and not that EGO converged.

#ifdef DEBUG
    Cout << "EGO Iteration " << globalIterCount << "\neif_star " << eif_star
	 << "\ndist_cstar "  << dist_cstar      << "\ndist_convergence_cntr "
	 << dist_convergence_cntr << '\n';
#endif //DEBUG

    if ( dist_convergence_cntr >= dist_convergence_limit ||
	 eif_convergence_cntr  >= eif_convergence_limit || 
	 globalIterCount       >= maxIterations )
      approx_converged = true;
    else {
      // Evaluate response_star_truth
      fHatModel.component_parallel_mode(TRUTH_MODEL);
      iteratedModel.continuous_variables(c_vars);
      ActiveSet set = iteratedModel.current_response().active_set();
      set.request_values(dataOrder);
      iteratedModel.evaluate(set);
      IntResponsePair resp_star_truth(iteratedModel.evaluation_id(),
				      iteratedModel.current_response());
    
      if (numNonlinearConstraints) {
	// Update the merit function parameters
	// Logic follows Conn, Gould, and Toint, section 14.4:
	const RealVector& fns_star_truth
	  = resp_star_truth.second.function_values();
	Real norm_cv_star = std::sqrt(constraint_violation(fns_star_truth, 0.));
	if (norm_cv_star < etaSequence)
	  update_augmented_lagrange_multipliers(fns_star_truth);
	else
	  update_penalty();
      }

      // Update the GP approximation
      fHatModel.append_approximation(vars_star, resp_star_truth, true);
    }

  } // end approx convergence while loop

  // Set best variables and response for use by strategy level.
  // c_vars, fmin contain the optimal design 
  get_best_sample(); // pull optimal result from sample data
  bestVariablesArray.front().continuous_variables(varStar);
  bestResponseArray.front().function_values(truthFnStar);

  // restore in case of recursion
  effGlobalInstance = prev_instance;

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
    size_t num_data_pts = gp_data.size(), num_vars = fHatModel.cv();
    for (size_t j=0; j<num_data_pts; ++j) {
      samsOut << '\n';
      const RealVector& sams = gp_data.continuous_variables(j);
      for (size_t k=0; k<num_vars; k++)
	samsOut << std::setw(13) << sams[k] << ' ';
      samsOut << std::setw(13) << gp_data.response_function(j);
    }
    samsOut << std::endl;

    // Plotting the GP over a grid is intended for visualization and
    //   is therefore only available for 2D problems
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
      Real interval0 = (ubnd[0]-lbnd[0])/100.,
	   interval1 = (ubnd[1]-lbnd[1])/100.;
      for (size_t j=0; j<101; j++){
	test_pt[0] = lbnd[0] + float(j)*interval0;
	for (size_t k=0; k<101; k++){
	  test_pt[1] = lbnd[1] + float(k)*interval1;
      
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
	    Real m = augmented_lagrangian_merit(gp_fn,
	      iteratedModel.primary_response_fn_sense(),
	      iteratedModel.primary_response_fn_weights(),
	      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
	      origNonlinEqTargets);
	    RealVector merit(1);
	    merit[0] = m;

	    Real ei = expected_improvement(merit, test_pt);

	    eifOut << '\n' << std::setw(13) << test_pt[0] << ' ' 
		   << std::setw(13) << test_pt[1] << ' ' << std::setw(13) << ei;
	  }
	}
	gpOut  << std::endl;
	varOut << std::endl;
	if (i==numFunctions-1)
	  eifOut << std::endl;
      }
    }
  }
#endif //DEBUG_PLOTS
}


void EffGlobalMinimizer::
EIF_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = effGlobalInstance->fHatModel.approximation_variances(recast_vars);

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) { // return -EI since we are maximizing
    Real neg_ei = -effGlobalInstance->expected_improvement(means, variances);
    recast_response.function_value(neg_ei, 0);
  }
}


Real EffGlobalMinimizer::
expected_improvement(const RealVector& means, const RealVector& variances)
{
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

  // Calculate the expected improvement
  Real cdf, pdf;
  Real snv = (meritFnStar - mean); // standard normal variate
  if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
    //this will trap the denominator=0.0 case even if numerator=0.0
    pdf=0.0;
    cdf=(snv>0.0)?1.0:0.0;
  }
  else{
    snv/=stdv; 
    cdf = Pecos::NormalRandomVariable::std_cdf(snv);
    pdf = Pecos::NormalRandomVariable::std_pdf(snv);
  }

  Real ei  = (meritFnStar - mean)*cdf + stdv*pdf;

  return ei;
}


RealVector EffGlobalMinimizer::
expected_violation(const RealVector& means, const RealVector& variances)
{
  RealVector ev(numNonlinearConstraints);

  size_t i, cntr=0;
  // inequality constraints
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    const Real& mean = means[numUserPrimaryFns+i];
    const Real& stdv = std::sqrt(variances[numUserPrimaryFns+i]);
    const Real& lbnd = origNonlinIneqLowerBnds[i];
    const Real& ubnd = origNonlinIneqUpperBnds[i];
    if (lbnd > -bigRealBoundSize) {
      Real cdf, pdf;
      Real snv = (lbnd-mean);
      if(std::fabs(snv)>=std::fabs(stdv)*50.0){
	pdf=0.0;
	cdf=(snv>0.0)?1.0:0.0;
      }
      else{
	snv/=stdv; //now snv is the standard normal variate
	cdf = Pecos::NormalRandomVariable::std_cdf(snv);
	pdf = Pecos::NormalRandomVariable::std_pdf(snv);
      }
      ev[cntr++] = (lbnd-mean)*cdf + stdv*pdf;
    }
    if (ubnd < bigRealBoundSize) {
      Real cdf, pdf;
      Real snv = (ubnd-mean);
      if(std::fabs(snv)>=std::fabs(stdv)*50.0){
	pdf=0.0;
	cdf=(snv>0.0)?1.0:0.0;
      }
      else{
	snv/=stdv;
	cdf = Pecos::NormalRandomVariable::std_cdf(snv);
	pdf = Pecos::NormalRandomVariable::std_pdf(snv);
      }
      ev[cntr++] = (mean-ubnd)*(1.-cdf) + stdv*pdf;
    }
  }
  // equality constraints
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


void EffGlobalMinimizer::get_best_sample()
{
  // Pull the samples and responses from data used to build latest GP
  // to determine fnStar for use in the expected improvement function

  const Pecos::SurrogateData& gp_data_0 = fHatModel.approximation_data(0);

  size_t i, sam_star_idx = 0, num_data_pts = gp_data_0.points();
  Real fn, fn_star = DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    const RealVector& sams = gp_data_0.continuous_variables(i);

    fHatModel.continuous_variables(sams);
    fHatModel.evaluate();
    const RealVector& f_hat = fHatModel.current_response().function_values();
    fn = augmented_lagrangian_merit(f_hat,
      iteratedModel.primary_response_fn_sense(),
      iteratedModel.primary_response_fn_weights(), origNonlinIneqLowerBnds,
      origNonlinIneqUpperBnds, origNonlinEqTargets);

    if (fn < fn_star) {
      copy_data(sams, varStar);
      sam_star_idx   = i;
      fn_star        = fn;
      meritFnStar    = fn;
      truthFnStar[0] = gp_data_0.response_function(i);
    }
  }

  // update truthFnStar with all additional primary/secondary fns corresponding
  // to lowest merit function value
  for (i=1; i<numFunctions; ++i)
    truthFnStar[i]
      = fHatModel.approximation_data(i).response_function(sam_star_idx);
}


void EffGlobalMinimizer::update_penalty()
{
  // Logic follows Conn, Gould, and Toint, section 14.4, step 3
  //   CGT use mu *= tau with tau = 0.01 ->   r_p *= 50
  //   Rodriguez, Renaud, Watson:             r_p *= 10
  //   Robinson, Willcox, Eldred, and Haimes: r_p *= 5
  penaltyParameter *= 10.;
  //penaltyParameter = std::min(penaltyParameter, 1.e+20); // cap the max penalty?
  Real mu = 1./2./penaltyParameter; // conversion between r_p and mu penalties
  etaSequence = eta*std::pow(mu, alphaEta);

#ifdef DEBUG
  Cout << "Penalty updated: " << penaltyParameter << '\n'
       << "eta updated:     " << etaSequence      << '\n'
       << "Augmented Lagrange multipliers:\n" << augLagrangeMult;
#endif
}

} // namespace Dakota
