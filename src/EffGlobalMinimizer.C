/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EffGlobalMinimizer
//- Description: Implementation code for the EffGlobalMinimizer class
//- Owner:       Barron J Bichon, Vanderbilt University

#include "EffGlobalMinimizer.H"
#include "system_defs.h"
#include "data_io.h"
#include "NonDLHSSampling.H"
#include "RecastModel.H"
#include "DataFitSurrModel.H"
#include "DakotaApproximation.H"
#include "ProblemDescDB.H"
#include "DakotaGraphics.H"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.H"
#endif
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "pecos_stat_util.hpp"
#include <boost/lexical_cast.hpp>

//#define DEBUG
//#define DEBUG_PLOTS

namespace Dakota {

EffGlobalMinimizer* EffGlobalMinimizer::effGlobalInstance(NULL);

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL 2


// This constructor accepts a Model
EffGlobalMinimizer::EffGlobalMinimizer(Model& model): 
  SurrBasedMinimizer(model), setUpType("model"), dataOrder(1)
{
  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  // initialize augmented Lagrange multipliers
  size_t num_multipliers = numNonlinearEqConstraints;
  for (size_t i=0; i<numNonlinearIneqConstraints; i++) {
    if (origNonlinIneqLowerBnds[i] > -bigRealBoundSize) // g has a lower bound
      num_multipliers++;
    if (origNonlinIneqUpperBnds[i] <  bigRealBoundSize) // g has an upper bound
      num_multipliers++;
  }
  augLagrangeMult.resize(num_multipliers);
  augLagrangeMult = 0.;

  truthFnStar.resize(numFunctions);

  // Always build a global Gaussian process model.  No correction is needed.
  String approx_type = "global_gaussian", sample_reuse = "none", corr_type;
  UShortArray approx_order; // empty
  short corr_order = -1;
  if (probDescDB.get_bool("method.derivative_usage")) {
    if (gradientType != "none") dataOrder |= 2;
    if (hessianType  != "none") dataOrder |= 4;
  }
  // Use a hardwired minimal initial samples.
  int samples  = (numContinuousVars+1)*(numContinuousVars+2)/2,
      lhs_seed = probDescDB.get_int("method.random_seed");
  String sample_type, rng; // empty strings: use defaults
  //int symbols = samples; // symbols needed for DDACE

  Iterator dace_iterator;
  // The following uses on the fly derived ctor:
  dace_iterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
    samples, lhs_seed, rng, ACTIVE_UNIFORM), false);
  // only use derivatives if the user requested and they are available
  ActiveSet dace_set = dace_iterator.active_set(); // copy
  dace_set.request_values(dataOrder);
  dace_iterator.active_set(dace_set);

  // Construct f-hat using a GP approximation for each response function over
  // the active/design vars (same view as iteratedModel: not the typical All
  // view for DACE).
  //const Variables& curr_vars = iteratedModel.current_variables();
  fHatModel.assign_rep(new DataFitSurrModel(dace_iterator, iteratedModel,
    //curr_vars.view(), curr_vars.variables_components(),
    //iteratedModel.current_response().active_set(),
    approx_type, approx_order, corr_type, corr_order, dataOrder, sample_reuse),
    false);

  // *** TO DO: support scaling and other forced Recasts. ***
  //if (scaleFlag || multiObjFlag || ...)
  //  iteratedModel.init_communicators(maxConcurrency);

  // eifModel.init_communicators() recursion is currently sufficient for
  // fHatModel.  An additional fHatModel.init_communicators() call would be
  // motivated by special parallel usage of fHatModel below that is not
  // otherwise covered by the recursion.
  //fHatMaxConcurrency = maxConcurrency; // local derivative concurrency
  //fHatModel.init_communicators(fHatMaxConcurrency);

  // Following this ctor, Strategy::init_iterator() initializes the parallel
  // configuration for EffGlobalMinimizer + iteratedModel using
  // EffGlobalMinimizer's maxConcurrency.  During fHatModel construction above,
  // DataFitSurrModel::derived_init_communicators() initializes the parallel
  // configuration for dace_iterator + iteratedModel using dace_iterator's
  // maxConcurrency.  The only iteratedModel concurrency currently exercised
  // is that used by dace_iterator within the initial GP construction, but the
  // EffGlobalMinimizer maxConcurrency must still be set so as to avoid parallel
  // configuration errors resulting from avail_procs > max_concurrency within
  // Strategy::init_iterator().  A max of the local derivative concurrency and
  // the DACE concurrency is used for this purpose.
  maxConcurrency = std::max(maxConcurrency,
			    dace_iterator.maximum_concurrency());

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level within the run fn.
  SizetArray recast_vars_comps_total; // default: empty; no change in size
  eifModel.assign_rep(
    new RecastModel(fHatModel, recast_vars_comps_total, 1, 0, 0), 
    false);

  // must use alternate NoDB ctor chain
  int max_iterations = 10000, max_fn_evals = 50000;
  double min_box_size = 1.e-15, vol_box_size = 1.e-15;
#ifdef HAVE_NCSU
  approxSubProbMinimizer.assign_rep(new
				    NCSUOptimizer(eifModel, max_iterations, 
						  max_fn_evals, min_box_size, 
						  vol_box_size), false);
  eifModel.init_communicators(approxSubProbMinimizer.maximum_concurrency());
#else
  Cerr << "NCSU DIRECT is not available to optimize the GP subproblems. " 
       << "Aborting process. " << std::endl;
        abort_handler(-1);
#endif //HAVE_NCSU
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
//EffGlobalMinimizer::
//EffGlobalMinimizer(Model& model, const int& max_iter, const int& max_eval) :
//  SurrBasedMinimizer(NoDBBaseConstructor(), model), setUpType("model")
//{ maxIterations = max_iter; maxFunctionEvals = max_eval; }


EffGlobalMinimizer::~EffGlobalMinimizer() 
{
  // deallocate communicators for DIRECT on eifModel
  eifModel.free_communicators(approxSubProbMinimizer.maximum_concurrency());

  // eifModel.free_communicators() recursion is currently sufficient for
  // fHatModel.  An additional fHatModel.free_communicators() call would be
  // motivated by special parallel usage of fHatModel below that is not
  // otherwise covered by the recursion.
  //fHatModel.free_communicators(fHatMaxConcurrency);
}


void EffGlobalMinimizer::minimize_surrogates()
{
  if (setUpType=="model")
    minimize_surrogates_on_model();
  else if (setUpType=="user_functions") {
    //minimize_surrogates_on_user_functions();
    Cerr << "Error: bad setUpType in EffGlobalMinimizer::minimize_surrogates()."
	 << std::endl;
    abort_handler(-1);
  }
  else {
    Cerr << "Error: bad setUpType in EffGlobalMinimizer::minimize_surrogates()."
	 << std::endl;
    abort_handler(-1);
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
  eifModel.update_from_subordinate_model(); // recurse_flag = true

  // Build initial GP once for all response functions
  fHatModel.build_approximation();

  // Iterate until EGO converges
  size_t eif_convergence_cntr = 0,dist_convergence_cntr=0;
  sbIterNum = 0;
  bool approxConverged = false;
  convergenceTol = 1.e-12;
  maxIterations  = 25*numContinuousVars;
  RealVector prevCStar;
  prevCStar	= -DBL_MAX;
  while (!approxConverged) {
    sbIterNum++;

    // initialize EIF recast model
    Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
    BoolDequeArray nonlinear_resp_map(1, BoolDeque(numFunctions, true));
    primary_resp_map[0].resize(numFunctions);
    for (size_t i=0; i<numFunctions; i++)
      primary_resp_map[0][i] = i;
    RecastModel* eif_model_rep = (RecastModel*)eifModel.model_rep();
    eif_model_rep->initialize(vars_map, false, NULL, NULL,
      primary_resp_map, secondary_resp_map, nonlinear_resp_map,
      EIF_objective_eval, NULL);

    // determine fnStar from among sample data
    get_best_sample();

    // Execute GLOBAL search and retrieve results
    Cout << "\n>>>>> Initiating global optimization\n";
    // no summary output since on-the-fly constructed:
    approxSubProbMinimizer.run_iterator(Cout);
    const Variables&    vars_star = approxSubProbMinimizer.variables_results();
    const RealVector&   c_vars = vars_star.continuous_variables();
    const Response&     resp_star = approxSubProbMinimizer.response_results();
    const Real&         eif_star  = resp_star.function_value(0);

    // Get expected value for output
    fHatModel.continuous_variables(c_vars);
    fHatModel.compute_response();
    const RealVector& mean = fHatModel.current_response().function_values();
    Real aug_lag = augmented_lagrangian_merit(mean, origNonlinIneqLowerBnds,
      origNonlinIneqUpperBnds, origNonlinEqTargets);

    Cout << "\nResults of EGO iteration:\nFinal point =\n";
    write_data(Cout, c_vars);
    Cout << "Expected Improvement    =\n                     "
	 << std::setw(write_precision+7) << -eif_star
	 << "\n                     " << std::setw(write_precision+7)
	 << aug_lag << " [merit]\n";

#ifdef DEBUG
    RealVector variance = fHatModel.approximation_variances(c_vars);
    RealVector ev = expected_violation(mean,variance);
    RealVector stdv(numFunctions);
    for (size_t i=0; i<numFunctions; i++)
      stdv[i] = std::sqrt(variance[i]);
    std::cout << "\nexpected values    =\n" << mean
	      << "\nstandard deviation =\n" << stdv
    	      << "\nexpected violation =\n" << ev << std::endl;
#endif //DEBUG

    // Check for convergence based on max EIF
    if ( -eif_star < convergenceTol )
      eif_convergence_cntr++;

    // Check for convergence based in distance between successive points
    // if the dist between successive points is very small
    // there is no point in asking this to happen consecutively 
    // since the new training point will essentially be the prev
    // optimal point 

    Real dist_tol = 1.e-8, rdcstar = rel_change_rv(c_vars, prevCStar);
    // update prevCStar
    copy_data(c_vars, prevCStar);
    if ( rdcstar < dist_tol)
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
    Cout << "sboIterNum " << sbIterNum << "\n";
    Cout << "eif_star " << eif_star << "\n";
    Cout << "rdcstar " << rdcstar << "\n";  
    Cout << "dist_convergence_cntr " << dist_convergence_cntr << "\n";
#endif //DEBUG

    if ( dist_convergence_cntr == 1 || eif_convergence_cntr==2 || 
	 sbIterNum >= maxIterations )
      approxConverged = true;
    else {
      // Evaluate response_star_truth
      fHatModel.component_parallel_mode(TRUTH_MODEL);
      iteratedModel.continuous_variables(c_vars);
      ActiveSet set = iteratedModel.current_response().active_set();
      set.request_values(dataOrder);
      iteratedModel.compute_response(set);
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
	  fHatModel.compute_response();
	  const Response& gp_resp = fHatModel.current_response();
	  const RealVector& gp_fn = gp_resp.function_values();
	  
	  gpOut << '\n' << std::setw(13) << test_pt[0] << ' ' 
		<< std::setw(13) << test_pt[1] << ' ' << std::setw(13) << gp_fn[i];

	  RealVector variances = fHatModel.approximation_variances(test_pt);

	  varOut << '\n' << std::setw(13) << test_pt[0] << ' '
		 << std::setw(13) << test_pt[1] << ' ' << std::setw(13) << variances[i];

	  if (i==numFunctions-1) {
	    Real m = augmented_lagrangian_merit(gp_fn, origNonlinIneqLowerBnds, 
	      origNonlinIneqUpperBnds, origNonlinEqTargets);
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
    = effGlobalInstance->fHatModel.approximation_variances(
      recast_vars.continuous_variables());

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) { // return -EI since we are maximizing
    Real neg_ei = -effGlobalInstance->expected_improvement(means, variances);
    recast_response.function_value(neg_ei, 0);
  }
}


Real EffGlobalMinimizer::
expected_improvement(const RealVector& means, const RealVector& variances)
{
  Real mean = objective(means), stdv;
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
  Real snv = (meritFnStar - mean)/stdv, // standard normal variate
       cdf = Pecos::Phi(snv), pdf = Pecos::phi(snv),
       ei  = (meritFnStar - mean)*cdf + stdv*pdf;

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
      Real snv = (lbnd-mean)/stdv, cdf = Pecos::Phi(snv), pdf = Pecos::phi(snv);
      ev[cntr++] = (lbnd-mean)*cdf + stdv*pdf;
    }
    if (ubnd < bigRealBoundSize) {
      Real snv = (ubnd-mean)/stdv, cdf = Pecos::Phi(snv), pdf = Pecos::phi(snv);
      ev[cntr++] = (mean-ubnd)*(1.-cdf) + stdv*pdf;
    }
  }
  // equality constraints
  for (i=0; i<numNonlinearEqConstraints; i++) {
    const Real& mean = means[numUserPrimaryFns+numNonlinearIneqConstraints+i];
    const Real& stdv
      = std::sqrt(variances[numUserPrimaryFns+numNonlinearIneqConstraints+i]);
    const Real& zbar = origNonlinEqTargets[i];
    Real snv = (zbar-mean)/stdv, cdf = Pecos::Phi(snv), pdf = Pecos::phi(snv);
    ev[cntr++] = (zbar-mean)*(2.*cdf-1.) + 2.*stdv*pdf;
  }

  return ev;
}


void EffGlobalMinimizer::get_best_sample()
{
  // Pull the samples and responses from data used to build latest GP
  // to determine fnStar for use in the expected improvement function

  const Pecos::SurrogateData& gp_data_0 = fHatModel.approximation_data(0);

  size_t i, sam_star_idx = 0, num_data_pts = gp_data_0.size();
  Real fn, fn_star = DBL_MAX;
  for (i=0; i<num_data_pts; ++i) {
    const RealVector& sams = gp_data_0.continuous_variables(i);

    fHatModel.continuous_variables(sams);
    fHatModel.compute_response();
    const RealVector& f_hat = fHatModel.current_response().function_values();
    fn = augmented_lagrangian_merit(f_hat, origNonlinIneqLowerBnds,
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
