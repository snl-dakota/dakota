/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGlobalInterval.H"
#include "system_defs.h"
#include "data_io.h"
#include "NonDLHSSampling.H"
#include "DakotaModel.H"
#include "DakotaIterator.H"
#include "RecastModel.H"
#include "DataFitSurrModel.H"
#include "ProblemDescDB.H"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.H"
#endif
#include "pecos_stat_util.hpp"

//#define DEBUG

namespace Dakota {

// initialization of statics
NonDGlobalInterval* NonDGlobalInterval::nondGIInstance(NULL);


NonDGlobalInterval::NonDGlobalInterval(Model& model):
  NonDInterval(model), seedSpec(probDescDB.get_int("method.random_seed")),
  numSamples(probDescDB.get_int("method.samples")),
  rngName(probDescDB.get_string("method.random_number_generator")),
  allResponsesPerIter(false) // obtain all response values on each fn call
{
  // Use a hardwired minimal initial samples
  if (!numSamples) // use a default of #terms in a quadratic polynomial
    numSamples = (numContinuousVars+1)*(numContinuousVars+2)/2;
  String approx_type = "global_gaussian", corr_type, sample_type,
    sample_reuse = "none";
  UShortArray approx_order; 
  short corr_order = -1;

  // instantiate the Gaussian Process DataFit recursions

  // The following uses on the fly derived ctor:
  daceIterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
    numSamples, seedSpec, rngName, ACTIVE_UNIFORM), false);

  // Construct fHatModel using a GP approximation over the active/uncertain
  // vars (same view as iteratedModel: not the typical All view for DACE).
 
  // Construct f-hat using a GP approximation for each response function over
  // the active/design vars (same view as iteratedModel: not the typical All
  // view for DACE).
  //const Variables& curr_vars = iteratedModel.current_variables();
  fHatModel.assign_rep(new DataFitSurrModel(daceIterator,
    iteratedModel, //curr_vars.view(), curr_vars.variables_components(),
    //iteratedModel.current_response().active_set(),
    approx_type, approx_order, corr_type, corr_order, sample_reuse), false);

  // eifModel.init_communicators() recursion is currently sufficient for
  // fHatModel.  An additional fHatModel.init_communicators() call would be
  // motivated by special parallel usage of fHatModel below that is not
  // otherwise covered by the recursion.
  //fHatMaxConcurrency = maxConcurrency; // local derivative concurrency
  //fHatModel.init_communicators(fHatMaxConcurrency);

  // Following this ctor, Strategy::init_iterator() initializes the parallel
  // configuration for NonDGlobalInterval + iteratedModel using
  // NonDGlobalInterval's maxConcurrency.  During fHatModel construction
  // above, DataFitSurrModel::derived_init_communicators() initializes the
  // parallel configuration for daceIterator + iteratedModel using
  // daceIterator's maxConcurrency.  The only iteratedModel concurrency
  // currently exercised is that used by daceIterator within the initial GP
  // construction, but the NonDGlobalInterval maxConcurrency must still be
  // set so as to avoid parallel configuration errors resulting from avail_procs
  // > max_concurrency within Strategy::init_iterator().  A max of the local
  // derivative concurrency and the DACE concurrency is used for this purpose.
  maxConcurrency = std::max(maxConcurrency, daceIterator.maximum_concurrency());

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level within the run fn.
  eifModel.assign_rep(new RecastModel(fHatModel, 1, 0, 0), false);

  // instantiate the optimizer used to improve the GP
  int max_iter = 1000, max_eval = 10000;
  double min_box_size = 1.e-15, vol_box_size = 1.e-15;
#ifdef HAVE_NCSU  
  gpOptimizer.assign_rep(new NCSUOptimizer(eifModel, max_iter, max_eval, 
					   min_box_size, vol_box_size), false);
  eifModel.init_communicators(gpOptimizer.maximum_concurrency());
#else
  Cerr << "NCSU DIRECT Optimizer is not available to use to find the" 
       << " interval bounds from the GP model." << std::endl;
  abort_handler(-1);
#endif // HAVE_NCSU
}


NonDGlobalInterval::~NonDGlobalInterval()
{  
  // deallocate communicators for DIRECT on eifModel
  eifModel.free_communicators(gpOptimizer.maximum_concurrency());

  // eifModel.free_communicators() recursion is currently sufficient for
  // fHatModel.  An additional fHatModel.free_communicators() call would be
  // motivated by special parallel usage of fHatModel below that is not
  // otherwise covered by the recursion.
  //fHatModel.free_communicators(fHatMaxConcurrency);
}


void NonDGlobalInterval::quantify_uncertainty()
{
  // set the object instance pointer for use within static member functions
  NonDGlobalInterval* prev_instance = nondGIInstance;
  nondGIInstance = this;

  eifModel.update_from_subordinate_model();
  // Build initial GP once for all response functions
  fHatModel.build_approximation();
  RecastModel* eif_model_rep = (RecastModel*)eifModel.model_rep();

  Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
  primary_resp_map[0].resize(1);
  BoolDequeArray nonlinear_resp_map(1);
  nonlinear_resp_map[0] = BoolDeque(numFunctions, false);

  convergenceTol = 1.e-12;
  maxIterations  = 25*numContinuousVars;
  prevCStar.size(numIntervalVars);

  initialize(); // virtual fn

  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {

    primary_resp_map[0][0] = respFnCntr;
    nonlinear_resp_map[0][respFnCntr] = true;

    for (cellCntr=0; cellCntr<numCells; ++cellCntr) {

      set_cell_bounds(); // virtual fn for setting bounds for local min/max

      // Iterate until EGO converges
      distConvergenceCntr = eifConvergenceCntr = sbIterNum = 0;
      prevCStar	= -DBL_MAX;	
      approxConverged = false;
      while (!approxConverged) {
	++sbIterNum;

	// initialize EIF recast model
	eif_model_rep->initialize(vars_map, false, NULL, NULL, primary_resp_map,
				  secondary_resp_map, nonlinear_resp_map,
				  EIF_objective_min, NULL);

	// determine approxFnStar from minimum among sample data
	get_best_sample(false, true);

	// Execute GLOBAL search and retrieve results
	Cout << "\n>>>>> Initiating global minimization\n";
	// no summary output since on-the-fly constructed:
	gpOptimizer.run_iterator(Cout);

	// output iteration results, update convergence controls, and update GP
	post_process_gp_results();
      }
      get_best_sample(false, false); // pull truthFnStar from sample data
      post_process_cell_results(true); // virtual fn: post-process min

      // Iterate until EGO converges
      distConvergenceCntr = eifConvergenceCntr = sbIterNum = 0;
      prevCStar	= -DBL_MAX;	
      approxConverged = false;
      while (!approxConverged) {
	++sbIterNum;

	// initialize EIF recast model
	eif_model_rep->initialize(vars_map, false, NULL, NULL, primary_resp_map,
				  secondary_resp_map, nonlinear_resp_map,
				  EIF_objective_max, NULL);

	// determine approxFnStar from maximum among sample data
	get_best_sample(true, true);

	// Execute GLOBAL search
	Cout << "\n>>>>> Initiating global maximization\n";
	// no summary output since on-the-fly constructed:
	gpOptimizer.run_iterator(Cout);

	// output iteration results, update convergence controls, and update GP
	post_process_gp_results();
      }
      get_best_sample(true, false); // pull truthFnStar from sample data
      post_process_cell_results(false); // virtual fn: post-process max
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


void NonDGlobalInterval::post_process_gp_results()
{
  const Variables& vars_star = gpOptimizer.variables_results();
  const RealVector& c_vars_star = vars_star.continuous_variables();
  Cout << "\nResults of EGO minimization:\nFinal point             =\n";
  write_data(Cout, c_vars_star);

  const Response& resp_star_approx = gpOptimizer.response_results();
  Real eif_star = -resp_star_approx.function_values()[0];
  Cout << "Expected Improvement    =\n                     "
       << std::setw(write_precision+7) << eif_star << "\n";

  // GT: Check Euclidean distance of successive optimal points
  // as second criteria for convergence
  // EIF encourages exploration through the variance of the point on GP 
  // But if the new point is very close to the previous point
  // this represents a failing of the variance to go to zero when evaluated near
  // data points.
  Real dist_tol = 1e-8;	
  Real rdcstar = rel_change_c_star(c_vars_star, prevCStar);
  // update prevCStar
  copy_data(c_vars_star, prevCStar);

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
	
  // GT:
  // The only reason we introduce this additional level of convergence is the
  // hope that adding the midpoint will improve the GP enough so that
  // 1. non-monotonic convergence can be addressed by improving the quality of
  // the GP in the hope that if a better soln exists, it can be captured by the
  // new GP or
  // 2. we construct a 'better' GP in the hope that DIRECT will actually find a
  // soln pt.  Furthermore, we do not require this to occur on consecutive runs
  // because if indeed we are not truly converged (e.g. bad GP, non-monotonic
  // nature of EIF convergence), if we allow the midpoint to be added as another
  // training point
  // >> theoretically does not change GP and there are no gains to be had
  // >> we could also add variance to the system as an artifact of the 
  // >>   numerical implementation
  if  ( eif_star < convergenceTol )
    ++eifConvergenceCntr;

  if ( rdcstar < dist_tol)
    ++distConvergenceCntr;
  // if the dist between successive points is very small
  // there is no point in asking this to happen consecutively 
  // since the new training point will essentially be the prev
  // optimal point 
  // >> theoretically does not change GP and there are no gains to be had
  // >> we could also add variance to the system as an artifact of the 
  // >> 	numerical implementation

  if ( distConvergenceCntr == 1 || eifConvergenceCntr == 2 ||
       sbIterNum >= maxIterations )
    approxConverged = true;
  else {
    // Evaluate response_star_truth
    //fHatModel.component_parallel_mode(TRUTH_MODEL);
    iteratedModel.continuous_variables(c_vars_star);
    ActiveSet set = iteratedModel.current_response().active_set();
    // GT:
    // Get all responses per function evaluation
    // changing this might break some of the logic needed to determine
    // whether the inner loop surrogate needs to be reconstructed
    if (allResponsesPerIter)
      set.request_values(1);
    else
      { set.request_values(0); set.request_value(respFnCntr, 1); }

    iteratedModel.compute_response(set);
    const Response& resp_star_truth = iteratedModel.current_response();

    // Update the GP approximation
    fHatModel.append_approximation(vars_star, resp_star_truth, true);
  }
}


void NonDGlobalInterval::get_best_sample(bool find_max, bool eval_approx)
{ } // default is no-op


void NonDGlobalInterval::post_process_cell_results(bool minimize)
{ } // default is no-op


void NonDGlobalInterval::post_process_response_fn_results()
{ } // default is no-op


void NonDGlobalInterval::post_process_final_results()
{ } // default is no-op


void NonDGlobalInterval::
EIF_objective_min(const Variables& sub_model_vars, const Variables& recast_vars,
		  const Response& sub_model_response, Response& recast_response)
{
  // Means are passed in, but must retrieve variance from the GP
  const RealVector& means = sub_model_response.function_values();
  const RealVector& variances
    = nondGIInstance->fHatModel.approximation_variances(
      recast_vars.continuous_variables());

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) { // return -EI since we are maximizing
    const Real& mean = means[nondGIInstance->respFnCntr];
    Real stdv = sqrt(variances[nondGIInstance->respFnCntr]);
    const Real& approx_fn_star = nondGIInstance->approxFnStar;
    // Calculate the expected improvement
    Real  snv = (approx_fn_star - mean)/stdv, // standard normal variate
      Phi_snv = Pecos::Phi(snv), phi_snv = Pecos::phi(snv),
      ei      = (approx_fn_star - mean)*Phi_snv + stdv*phi_snv;
    recast_response.function_value(-ei, 0);
#ifdef DEBUG
    Cout << "(Evaluation,ApproxFnStar,Phi,phi,vars): (" << mean << "," 
	 << approx_fn_star << "," << Phi_snv << "," << phi_snv;
    RealVector eval_vars = recast_vars.continuous_variables();
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
    = nondGIInstance->fHatModel.approximation_variances(
      recast_vars.continuous_variables());

  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) { // return -EI to the minimizer
    Real mean = -means[nondGIInstance->respFnCntr],
         stdv = sqrt(variances[nondGIInstance->respFnCntr]);
    const Real& approx_fn_star = nondGIInstance->approxFnStar;
    // Calculate the expected improvement
    Real  snv = (-approx_fn_star-mean)/stdv, // standard normal variate   
      Phi_snv = Pecos::Phi(snv), phi_snv = Pecos::phi(snv),
      ei      = (-approx_fn_star-mean)*Phi_snv + stdv*phi_snv;
    // minimize -EIF -> maximize EIF
    recast_response.function_value(-ei, 0);
#ifdef DEBUG
    Cout << "(Evaluation,ApproxFnStar,Phi,phi,vars): (" << mean << "," 
	 << approx_fn_star << "," << Phi_snv << "," << phi_snv;
    RealVector eval_vars = recast_vars.continuous_variables();
    for (size_t i=0; i < eval_vars.length(); i++)
      Cout << ", " << eval_vars[i];
    Cout << ")\n";	
#endif	
  }
}

} // namespace Dakota
