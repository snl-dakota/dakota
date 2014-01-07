/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalReliability
//- Description: Implementation code for NonDGlobalReliability class
//- Owner:       Barron Bichon, Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "NonDGlobalReliability.hpp"
#include "NonDLHSSampling.hpp"
//#include "DDACEDesignCompExp.hpp"
//#include "FSUDesignCompExp.hpp"
#include "NonDAdaptImpSampling.hpp"
//#ifdef DAKOTA_COLINY
//#include "COLINOptimizer.hpp"
//#endif
#include "RecastModel.hpp"
#include "DataFitSurrModel.hpp"
#include "DakotaApproximation.hpp"
#include "ProblemDescDB.hpp"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#include "pecos_stat_util.hpp"
#include <boost/lexical_cast.hpp>

//#define DEBUG
//#define DEGUG_PLOTS

static const char rcsId[] = "@(#) $Id: NonDGlobalReliability.cpp 4058 2006-10-25 01:39:40Z mseldre $";

extern "C" {
#ifdef DAKOTA_F90
  #if defined(HAVE_CONFIG_H) && !defined(DISABLE_DAKOTA_CONFIG_H)

  // Deprecated; continue to support legacy, clashing macros for ONE RELEASE
  #define BVLS_WRAPPER_FC FC_FUNC_(bvls_wrapper,BVLS_WRAPPER)
  void BVLS_WRAPPER_FC( Dakota::Real* a, int& m, int& n, Dakota::Real* b,
		        Dakota::Real* bnd, Dakota::Real* x, Dakota::Real& rnorm,
		        int& nsetp, Dakota::Real* w, int* index, int& ierr );
  #else

  // Use the CMake-generated fortran name mangling macros (eliminate warnings)
  #include "dak_f90_config.h"
  #define BVLS_WRAPPER_FC DAK_F90_GLOBAL_(bvls_wrapper,BVLS_WRAPPER)
  void BVLS_WRAPPER_FC( Dakota::Real* a, int& m, int& n, Dakota::Real* b,
		        Dakota::Real* bnd, Dakota::Real* x, Dakota::Real& rnorm,
		        int& nsetp, Dakota::Real* w, int* index, int& ierr );
  #endif // HAVE_CONFIG_H and !DISABLE_DAKOTA_CONFIG_H
#endif // DAKOTA_F90
}

namespace Dakota {

// initialization of statics
NonDGlobalReliability* NonDGlobalReliability::nondGlobRelInstance(NULL);

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL     2

using std::setw;

NonDGlobalReliability::NonDGlobalReliability(Model& model): 
  NonDReliability(model), meritFunctionType(AUGMENTED_LAGRANGIAN_MERIT),
  dataOrder(1)
{
  const String& mpp_search
    = probDescDB.get_string("method.nond.reliability_search_type");
  if (mpp_search == "egra_x")
    mppSearchType = EGRA_X;
  else if (mpp_search == "egra_u")
    mppSearchType = EGRA_U;
  else {
    Cerr << "Error: only x-space and u-space EGRA are currently supported in "
	 << "global_reliability."<< std::endl;
    abort_handler(-1); 
  }

  // standard reliability indices are not defined and should be precluded via
  // the input spec.  requestedRelLevels cannot be used for empty() tests.
  if (!probDescDB.get_rva("method.nond.reliability_levels").empty() ||
      respLevelTarget == RELIABILITIES) {
    Cerr << "Error: reliability indices are not defined for global reliability "
	 << "methods.  Use generalized reliability instead." << std::endl;
    abort_handler(-1); 
  }

  // requestedProbLevels & requestedGenRelLevels are not yet supported
  // since PMA EGRA is currently a research issue.  requestedProbLevels
  // and requestedGenRelLevels cannot be used for empty() tests.
  if (!probDescDB.get_rva("method.nond.probability_levels").empty() ||
      !probDescDB.get_rva("method.nond.gen_reliability_levels").empty()) {
    Cerr << "Error: Inverse reliability mappings not currently supported in "
	 << "global_reliability."<< std::endl;
    abort_handler(-1); 
  }

#ifndef DAKOTA_F90
  if (meritFunctionType == LAGRANGIAN_MERIT) {
    Cerr << "Error: F90 required for standard Lagrangian merit function in "
	 << "global_reliability."<< std::endl;
    abort_handler(-1); 
  }
#endif // DAKOTA_F90

  // Size the output arrays.  Relative to sampling methods, the output storage
  // for reliability methods is more substantial since there may be differences
  // between requested and computed levels for the same measure (the request is
  // not always achieved) and since probability and reliability are carried
  // along in parallel (due to their direct correspondence).
  size_t i, j;
  for (i=0; i<numFunctions; i++) {
    size_t num_levels = requestedRespLevels[i].length() + 
      requestedProbLevels[i].length() + requestedGenRelLevels[i].length();
    computedRespLevels[i].resize(num_levels);
    computedProbLevels[i].resize(num_levels);
    computedGenRelLevels[i].resize(num_levels);
  }

  // The Gaussian process model of the limit state in u-space [G-hat(u)] is 
  // constructed here one time.
  
  // Always build a global Gaussian process model.  No correction is needed.
  String approx_type = "global_kriging";
  if (probDescDB.get_short("method.nond.emulator") == GAUSSIAN_PROCESS)
    approx_type = "global_gaussian";

  String sample_type;
  UShortArray approx_order; // not used for GP/kriging
  short corr_order = -1, corr_type = NO_CORRECTION,
    active_view = iteratedModel.current_variables().view().first;
  if (probDescDB.get_bool("method.derivative_usage")) {
    if (approx_type == "global_gaussian") {
      Cerr << "\nError: efficient_global does not support gaussian_process "
	   << "when derivatives present; use kriging instead." << std::endl;
      abort_handler(-1);
    }
    if (gradientType != "none") dataOrder |= 2;
    if (hessianType  != "none") dataOrder |= 4;
  }
  String sample_reuse
    = (active_view == RELAXED_ALL || active_view == MIXED_ALL) ? "all" : "none";

  // Use a hardwired minimal initial samples
  int samples  = (numContinuousVars+1)*(numContinuousVars+2)/2,
      lhs_seed = probDescDB.get_int("method.random_seed");
  const String& rng = probDescDB.get_string("method.random_number_generator");
  bool vary_pattern = false; // for consistency across outer loop invocations
  // get point samples file
  const String& import_pts_file
    = probDescDB.get_string("method.import_points_file");
  if (!import_pts_file.empty())
    { samples = 0; sample_reuse = "all"; }

  //int symbols = samples; // symbols needed for DDACE
  Iterator dace_iterator;
  // instantiate the Nataf Recast and Gaussian Process DataFit recursions
  if (mppSearchType == EGRA_X) { // Recast( DataFit( iteratedModel ) )

    // For additional generality, could develop on the fly envelope ctor:
    //Iterator dace_iterator(iteratedModel, dace_method, ...);

    // The following uses on the fly derived ctor:
    dace_iterator.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
      samples, lhs_seed, rng, vary_pattern, ACTIVE_UNIFORM), false);
    //String dace_method = "lhs";
    //dace_iterator.assign_rep(new DDACEDesignCompExp(iteratedModel, samples,
    //                         symbols, lhs_seed, dace_method), false);
    //String dace_method = "fsu_hammersley";
    //dace_iterator.assign_rep(new FSUDesignCompExp(iteratedModel, samples,
    //                         lhs_seed, dace_method), false);

    // Construct g-hat(x) using a GP approximation over the active/uncertain
    // vars (same view as iteratedModel: not the typical All view for DACE).
    // For RBDO with GP over {u}+{d}, view set to All from all_variables spec.
    Model g_hat_x_model;
    IntSet surr_fn_indices; // Only want functions with requested levels
    ActiveSet set = iteratedModel.current_response().active_set();// copy
    set.request_values(0);
    for (i=0; i<numFunctions; ++i)
      if (!computedRespLevels[i].empty()) // sized to total req levels above
    	{ set.request_value(dataOrder, i); surr_fn_indices.insert(i); }
    dace_iterator.active_set(set);
    //const Variables& curr_vars = iteratedModel.current_variables();
    g_hat_x_model.assign_rep(new DataFitSurrModel(dace_iterator, iteratedModel,
      //curr_vars.view(), curr_vars.variables_components(),
      //iteratedModel.current_response().active_set(),
      approx_type, approx_order, corr_type, corr_order, dataOrder, outputLevel,
      sample_reuse, probDescDB.get_string("method.export_points_file"),
      probDescDB.get_bool("method.export_points_file_annotated"),
      import_pts_file,
      probDescDB.get_bool("method.import_points_file_annotated")), false);
    g_hat_x_model.surrogate_function_indices(surr_fn_indices);

    // Recast g-hat(x) to G-hat(u)
    transform_model(g_hat_x_model, uSpaceModel, true, 5.); // global bounds
  }
  else { // DataFit( Recast( iteratedModel ) )

    // Recast g(x) to G(u)
    Model g_u_model;
    transform_model(iteratedModel, g_u_model, true, 5.); // global bounds

    // For additional generality, could develop on the fly envelope ctor:
    //Iterator dace_iterator(g_u_model, dace_method, ...);

    // The following use on-the-fly derived ctors:
    dace_iterator.assign_rep(new NonDLHSSampling(g_u_model, sample_type,
      samples, lhs_seed, rng, vary_pattern, ACTIVE_UNIFORM), false);
    //String dace_method = "lhs";
    //dace_iterator.assign_rep(new DDACEDesignCompExp(g_u_model, samples,
    //                         symbols, lhs_seed, dace_method), false);
    //String dace_method = "fsu_hammersley";
    //dace_iterator.assign_rep(new FSUDesignCompExp(g_u_model, samples,
    //                         lhs_seed, dace_method), false);

    // Construct G-hat(u) using a GP approximation over the active/uncertain
    // variables (using the same view as iteratedModel/g_u_model: not the
    // typical All view for DACE).
    // For RBDO with GP over {u}+{d}, view set to All from all_variables spec.
    IntSet surr_fn_indices; // Only want functions with requested levels
    ActiveSet set = iteratedModel.current_response().active_set();// copy
    set.request_values(0);
    for (i=0; i<numFunctions; ++i)
      if (!computedRespLevels[i].empty()) // sized to total req levels above
    	{ set.request_value(dataOrder, i); surr_fn_indices.insert(i); }
    dace_iterator.active_set(set);

    //const Variables& g_u_vars = g_u_model.current_variables();
    uSpaceModel.assign_rep(new DataFitSurrModel(dace_iterator, g_u_model,
      //g_u_vars.view(), g_u_vars.variables_components(),
      //g_u_model.current_response().active_set(),
      approx_type, approx_order, corr_type, corr_order, dataOrder, outputLevel,
      sample_reuse, probDescDB.get_string("method.export_points_file"),
      probDescDB.get_bool("method.export_points_file_annotated"),
      import_pts_file,
      probDescDB.get_bool("method.import_points_file_annotated")), false);
    uSpaceModel.surrogate_function_indices(surr_fn_indices);
  }

  // mppModel.init_communicators() recursion is currently sufficient for
  // uSpaceModel.  An additional uSpaceModel.init_communicators() call would be
  // motivated by special parallel usage of uSpaceModel below that is not
  // otherwise covered by the recursion.
  //uSpaceMaxConcurrency = maxConcurrency; // local derivative concurrency
  //uSpaceModel.init_communicators(uSpaceMaxConcurrency);

  // Following this ctor, Strategy::init_iterator() initializes the parallel
  // configuration for NonDGlobalReliability + iteratedModel using
  // NonDGlobalReliability's maxConcurrency.  During uSpaceModel construction
  // above, DataFitSurrModel::derived_init_communicators() initializes the
  // parallel configuration for dace_iterator + iteratedModel using
  // dace_iterator's maxConcurrency.  The only iteratedModel concurrency
  // currently exercised is that used by dace_iterator within the initial GP
  // construction, but the NonDGlobalReliability maxConcurrency must still be
  // set so as to avoid parallel configuration errors resulting from
  // avail_procs > max_concurrency within Strategy::init_iterator().  A max
  // of the local derivative concurrency and the DACE concurrency is used for
  // this purpose.
  maxConcurrency = std::max(maxConcurrency,dace_iterator.maximum_concurrency());

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor.  The RIA/PMA expected improvement/
  // expected feasibility formulations may vary with the level requests, so
  // the recast fn pointers are reset for each level within the run fn.
  SizetArray recast_vars_comps_total; // default: empty; no change in size
  mppModel.assign_rep(
    new RecastModel(uSpaceModel, recast_vars_comps_total, 1, 0, 0),
    false);

  // For formulations with one objective and one equality constraint,
  // use the following instead:
  //mppModel.assign_rep(new RecastModel(uSpaceModel, 1, 1, 0), false);
  //RealVector nln_eq_targets(1, 0.);
  //mppModel.nonlinear_eq_constraint_targets(nln_eq_targets);

  // must use alternate NoDB ctor chain
  int max_iter = 1000, max_eval = 10000;
  double min_box_size = 1.e-15, vol_box_size = 1.e-15;
#ifdef HAVE_NCSU  
  mppOptimizer.assign_rep(new NCSUOptimizer(mppModel, max_iter, max_eval,
					    min_box_size, vol_box_size), false);
  //#ifdef DAKOTA_COLINY
  //int coliny_seed = 0; // system-generated, for now
  //mppOptimizer.assign_rep(new
  //  COLINOptimizer<coliny::DIRECT>(mppModel, coliny_seed), false);
  //mppOptimizer.assign_rep(new
  //  COLINOptimizer<coliny::EAminlp>(mppModel, coliny_seed), false);
  //#endif
  mppModel.init_communicators(mppOptimizer.maximum_concurrency());
#else
  Cerr << "NCSU DIRECT Optimizer is not available to use in the MPP search " 
       << "in global reliability optimization:  aborting process." << std::endl;
        abort_handler(-1);
#endif //HAVE_NCSU

  // The importance sampler uses uSpaceModel (without additional recasting)
  // and may be constructed here.  Thus, NonDGlobal applies integration
  // refinement to the G-hat(u) surrogate.  Behavior needs to be repeatable
  // and AIS is not part of the EGRA spec: either reuse lhs_seed or hardwire.
  int refine_samples = 1000, refine_seed = 123457;
  // these flags control if/when space transformation is needed in the sampler
  bool x_model_flag = false, bounded_model = true;
  bool x_data_flag = (mppSearchType == EGRA_X) ? true : false;
  integrationRefinement = MMAIS; vary_pattern = true;

  importanceSampler.assign_rep(new
    NonDAdaptImpSampling(uSpaceModel, sample_type, refine_samples, refine_seed,
			 rng, vary_pattern, integrationRefinement, cdfFlag,
			 x_data_flag, x_model_flag, bounded_model), false);

  uSpaceModel.init_communicators(importanceSampler.maximum_concurrency());
}


NonDGlobalReliability::~NonDGlobalReliability()
{
  // deallocate communicators for MMAIS on uSpaceModel
  uSpaceModel.free_communicators(importanceSampler.maximum_concurrency());

  // deallocate communicators for DIRECT on mppModel
  mppModel.free_communicators(mppOptimizer.maximum_concurrency());

  // mppModel.free_communicators() recursion is currently sufficient for
  // uSpaceModel.  An additional uSpaceModel.free_communicators() call would
  // be motivated by special parallel usage of uSpaceModel below that is not
  // otherwise covered by the recursion.
  //uSpaceModel.free_communicators(uSpaceMaxConcurrency);
}


void NonDGlobalReliability::quantify_uncertainty()
{
  // initialize the random variable arrays and the correlation Cholesky factor
  initialize_random_variable_parameters();
  natafTransform.transform_correlations();

  // set the object instance pointer for use within static member functions
  NonDGlobalReliability* prev_grel_instance = nondGlobRelInstance;
  nondGlobRelInstance = this;

  // Optimize the GP for all levels and then use MAIS for _all_ levels
  optimize_gaussian_process();
  importance_sampling();
  numRelAnalyses++;

  // restore in case of recursion
  nondGlobRelInstance = prev_grel_instance;
}


void NonDGlobalReliability::optimize_gaussian_process()
{
  // now that variables/labels/bounds/targets have flowed down at run-time from
  // any higher level recursions, propagate them up the instantiate-on-the-fly
  // Model recursion so that they are correct when they propagate back down.
  mppModel.update_from_subordinate_model(); // recurse_flag = true

  if (mppSearchType == EGRA_X) {
    // assign non-default global variable bounds for use in DACE.
    // This does not affect any uncertain variable distribution bounds.
    // Note 1: the interval defined in x-space will be sampled uniformly by
    // DACE methods, which could result in very irregular coverage in u-space.
    // It would be better to sample uniformly in u-space.
    // Note 2: it would be preferable to set this up in the constructor.  The
    // EGRA_U case can do this by setting the bounds in the actualModel of the
    // DataFitSurrModel and then having them copied in the DataFitSurrModel
    // ctor.  The EGRA_X case needs trans_U_to_X, which isn't available until
    // after initialize_random_variable_parameters() is executed at run time.
    // Therefore, this case sets the bounds for the DataFitSurrModel, which are
    // then propagated to actualModel in DataFitSurrModel::update_actual_model()
    RealVector u_l_bnds(numContinuousVars, false), x_l_bnds,
               u_u_bnds(numContinuousVars, false), x_u_bnds;
    u_l_bnds = -5.; u_u_bnds = 5.;
    for (size_t i=0; i<numContDesVars; i++)
      { u_l_bnds[i] = -1.; u_u_bnds[i] =  1.; }
    for (size_t i=numContDesVars+numContAleatUncVars; i<numContinuousVars; i++)
      { u_l_bnds[i] = -1.; u_u_bnds[i] =  1.; }
    natafTransform.trans_U_to_X(u_l_bnds, x_l_bnds);
    natafTransform.trans_U_to_X(u_u_bnds, x_u_bnds);
    Model& g_hat_x_model = uSpaceModel.subordinate_model();
    g_hat_x_model.continuous_lower_bounds(x_l_bnds);
    g_hat_x_model.continuous_upper_bounds(x_u_bnds);
  }
  else {
    NonDLHSSampling* lhs_sampler_rep
      = (NonDLHSSampling*)uSpaceModel.subordinate_iterator().iterator_rep();
    // pass x-space data so that u-space Models can perform inverse transforms
    lhs_sampler_rep->initialize_random_variables(natafTransform);
  }

  // Build initial GP once for all response functions
  uSpaceModel.build_approximation();
  
  // Loop over each response function in the responses specification.  It is
  // important to note that the MPP iteration is different for each response 
  // function, and it is not possible to combine the model evaluations for
  // multiple response functions.
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {

    // The most general case is to allow a combination of response, probability,
    // and reliability level specifications for each response function.
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           gl_len = requestedGenRelLevels[respFnCount].length(),
           num_levels = rl_len + pl_len + gl_len;

    // Loop over response/probability/reliability levels
    for (levelCount=0; levelCount<num_levels; levelCount++) {

      // The rl_len response levels are performed first using the RIA
      // formulation, followed by the pl_len probability levels and the
      // gl_len generalized reliability levels using the PMA formulation.
      bool ria_flag = (levelCount < rl_len) ? true : false;
      if (ria_flag) {
        requestedTargetLevel = requestedRespLevels[respFnCount][levelCount];
	Cout << "\n>>>>> Reliability Index Approach (RIA) for response level "
	     << levelCount+1 << " = " << requestedTargetLevel << '\n';
      }
      else if (levelCount < rl_len + pl_len) { 
	size_t index = levelCount-rl_len;
	Real p = requestedProbLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for probability "
	     << "level " << index+1 << " = " << p << '\n';
	requestedTargetLevel = -Pecos::Phi_inverse(p);
	Real gen_beta_cdf = (cdfFlag) ?
	  requestedTargetLevel : -requestedTargetLevel;
	pmaMaximizeG = (gen_beta_cdf < 0.);
      }
      else {
	size_t index = levelCount-rl_len-pl_len;
	requestedTargetLevel = requestedGenRelLevels[respFnCount][index];
	Cout << "\n>>>>> Performance Measure Approach (PMA) for reliability "
	     << "level " << index+1 << " = " << requestedTargetLevel << '\n';
	Real gen_beta_cdf = (cdfFlag) ?
	  requestedTargetLevel : -requestedTargetLevel;
	pmaMaximizeG = (gen_beta_cdf < 0.);
      }

      bool pma_aug_lag_flag
	= (!ria_flag && meritFunctionType == AUGMENTED_LAGRANGIAN_MERIT);
      if (pma_aug_lag_flag) {
	augLagrangeMult         = 0.;      penaltyParameter    = 1.;
	lastConstraintViolation = DBL_MAX; lastIterateAccepted = false;
      }

      // Iterate until EGRA converges
      approxIters = 0;
      approxConverged = false;
      while (!approxConverged) {

	approxIters++;

	// construct global optimizer and its EI/EF recast model
	Sizet2DArray vars_map, primary_resp_map, secondary_resp_map;
	BoolDequeArray nonlinear_resp_map(1, BoolDeque(1, true));
	RecastModel* mpp_model_rep = (RecastModel*)mppModel.model_rep();
	if (ria_flag) {
	  // Standard RIA : min u'u  s.t. g = z_bar
	  // use RIA evaluators to recast g into global opt subproblem
	  //primary_resp_map.reshape(1);   // one objective, no contributors
	  //secondary_resp_map.reshape(1); // one constraint, one contributor
	  //secondary_resp_map[0].reshape(1);
	  //secondary_resp_map[0][0] = respFnCount;
	  //BoolDequeArray nonlinear_resp_map(2);
	  //nonlinear_resp_map[1] = BoolDeque(1, false);
	  //mpp_model_rep->initialize(vars_map, false, NULL, NULL,
	  //  primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	  //  RIA_objective_eval, RIA_constraint_eval);

	  // EFF formulation : max EFF  s.t. bound constraints
	  // use EFF evaluators to recast g into global opt subproblem
	  primary_resp_map.resize(1);
	  primary_resp_map[0].resize(1);
	  primary_resp_map[0][0] = respFnCount;
	  mpp_model_rep->initialize(vars_map, false, NULL, NULL,
	    primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	    EFF_objective_eval, NULL);
	}
	else {
	  // Standard PMA : min/max g  s.t. u'u = beta_bar^2
	  // use PMA evaluators to recast g into global opt subproblem
	  //void (*set_map) (const ShortArray& recast_asv,
	  //                 ShortArray& sub_model_asv) =
	  //  (integrationOrder == 2) ? PMA2_set_mapping : NULL;
	  //primary_resp_map.reshape(1);   // one objective, one contributor
	  //primary_resp_map[0].reshape(1);
	  //primary_resp_map[0][0] = respFnCount;
	  //secondary_resp_map.reshape(1); // one constraint, no contributors
	  //BoolDequeArray nonlinear_resp_map(2);
	  //nonlinear_resp_map[0] = BoolDeque(1, false);
	  //mpp_model_rep->initialize(vars_map, false, NULL, set_map,
	  //  primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	  //  PMA_objective_eval, PMA_constraint_eval);

	  // EIF formulation : max Phi(EIF) [merit function on EIF]
	  // determine fnStar from among sample data
	  get_best_sample();
	  // use EIF evaluators to recast g into global opt subproblem
	  primary_resp_map.resize(1);
	  primary_resp_map[0].resize(1);
	  primary_resp_map[0][0] = respFnCount;
	  mpp_model_rep->initialize(vars_map, false, NULL, NULL,
	    primary_resp_map, secondary_resp_map, nonlinear_resp_map,
	    EIF_objective_eval, NULL);
	}

	// Execute GLOBAL search and retrieve u-space results
	Cout << "\n>>>>> Initiating global reliability optimization\n";
	// no summary output since on-the-fly constructed:
	mppOptimizer.run_iterator(Cout);
	// Use these two lines for COLINY optimizers
	//const VariablesArray& vars_star
	//  = mppOptimizer.variables_array_results();
	//const RealVector& c_vars_u = vars_star[0].continuous_variables();
	// Use these two lines for NCSU DIRECT
	const Variables& vars_star = mppOptimizer.variables_results();
	const RealVector& c_vars_u = vars_star.continuous_variables();

	// Get expected value at u* for output
	uSpaceModel.continuous_variables(c_vars_u);
	uSpaceModel.compute_response();
	const RealVector& g_hat_fns
	  = uSpaceModel.current_response().function_values();

	// Re-evaluate the expected improvement/feasibility at vars_star
	Real beta_star = 0.,/* TO DO */  exp_fns_star = (ria_flag) ?
	  expected_feasibility(g_hat_fns, vars_star) :
	  expected_improvement(g_hat_fns, vars_star);
    
	Cout << "\nResults of EGRA iteration:\nFinal point (u-space)   =\n";
	write_data(Cout, c_vars_u);
	if (ria_flag) {
	  Cout << "Expected Feasibility    =\n                     "
	       << setw(write_precision+7) << -exp_fns_star << "\n"
	       << "                     " << setw(write_precision+7)
	       << g_hat_fns[respFnCount] - requestedTargetLevel
	       << " [G_hat(u) - z]\n";
	//     << "                     " << setw(write_precision+7)
	//     << beta_star                 << " [beta*]\n";
	//Cout << "RIA optimum             =\n                     "
	//     << setw(write_precision+7) << exp_fns_star << " [u'u]\n"
	//     << "                     " << setw(write_precision+7)
	//     << exp_fns_star[1] << " [G(u) - z]\n";
	}
	else {
	  // Calculate beta^2 for output (and aug_lag update)
	  Cout << "Expected Improvement    =\n                     "
	       << setw(write_precision+7)          << -exp_fns_star << "\n"
	       << "                     "          << setw(write_precision+7)
	       << beta_star - requestedTargetLevel << " [beta* - bar-beta*]\n"
	       << "                     "          << setw(write_precision+7)
	       << g_hat_fns[respFnCount]           << " [G_hat(u)]\n";
	//Cout << "PMA optimum             =\n                     "
	//     << setw(write_precision+7) << exp_fns_star << " [";
	//if (pmaMaximizeG) Cout << '-';
	//Cout << "G(u)]\n                     " << setw(write_precision+7)
	//     << exp_fns_star[1] << " [u'u - B^2]\n";
	}

	// Update parameters for the augmented Lagrangian merit function
	if (pma_aug_lag_flag) {
	  // currently only used for PMA with EIF
	  Real c_violation = beta_star - requestedTargetLevel;
	  if (c_violation < lastConstraintViolation)
	    lastIterateAccepted = true;
	  lastConstraintViolation = c_violation;
	}
    
	// Check for convergence based on max EIF/EFF
	convergenceTol = .001;
        if (maxIterations < 0) 
          maxIterations  = 25*numContinuousVars;
	if (approxIters >= maxIterations || -exp_fns_star < convergenceTol)
	  approxConverged = true;
	else {
	  // Evaluate response_star_truth
	  uSpaceModel.component_parallel_mode(TRUTH_MODEL);
	  RealVector c_vars_x;
	  natafTransform.trans_U_to_X(c_vars_u, c_vars_x);
	  iteratedModel.continuous_variables(c_vars_x);
	  ActiveSet set = iteratedModel.current_response().active_set();
	  set.request_values(0); set.request_value(dataOrder, respFnCount);
	  iteratedModel.compute_response(set);
	  IntResponsePair resp_star_truth(iteratedModel.evaluation_id(),
					  iteratedModel.current_response());

	  // Update the GP approximation
	  if (mppSearchType == EGRA_X) // update with x-space current vars
	    uSpaceModel.append_approximation(
	      iteratedModel.current_variables(), resp_star_truth, true);
	  else                         // update with u-space vars_star
	    uSpaceModel.append_approximation(vars_star, resp_star_truth, true);
	}
      } // end approx convergence while loop
      
      if (ria_flag)
	Cout << "\n<<<<< GP model has converged for response level ";
      else
 	Cout << "\n<<<<< GP model has converged for response level ";
      Cout << levelCount+1 << " = " << requestedTargetLevel << '\n';
     
    } // end loop over levels

    if (num_levels)
      Cout << "\n<<<<< GP model has converged for all requested levels of "
	   << "response function " << respFnCount+1 << '\n';

#ifdef DEBUG
    // DEBUG - output set of samples used to build the GP
    // If problem is 2d, output a grid of points on the GP, variance, 
    //   eff, and truth (if requested)
    if (num_levels) {  // can only plot if GP was built!
      std::string samsfile("egra_sams");
      std::string tag = "_" + boost::lexical_cast<std::string>(respFnCount+1) +
                        ".out";
      samsfile += tag;
      std::ofstream samsOut(samsfile.c_str(),std::ios::out);
      samsOut << std::scientific;
      const Pecos::SurrogateData& gp_data
	= uSpaceModel.approximation_data(respFnCount);
      size_t num_data_pts = gp_data.size(), num_vars = uSpaceModel.cv();
      for (size_t i=0; i<num_data_pts; ++i) {
	const RealVector& sams = gp_data.continuous_variables(i); // view
	Real true_fn = gp_data.response_function(i);
	
	if (mppSearchType == EGRA_X) {
	  RealVector sams_u(num_vars);
	  natafTransform.trans_X_to_U(sams,sams_u);
	  
	  samsOut << '\n';
	  for (size_t j=0; j<num_vars; j++)
	    samsOut << setw(13) << sams_u[j] << ' ';
	  samsOut<< setw(13) << true_fn;
	}
	else {
	  samsOut << '\n';
	  for (size_t j=0; j<num_vars; j++)
	    samsOut << setw(13) << sams[j] << ' ';
	  samsOut<< setw(13) << true_fn;
	}
      }
      samsOut << std::endl;

#ifdef DEBUG_PLOTS
      // Plotting the GP, etc over a grid is intended for visualization and
      //   is therefore only available for 2D problems
      if (num_vars==2) {
	bool true_plot = false;
        std::string truefile("egra_true"), gpfile("egra_gp"),
                    varfile("egra_var"), efffile("egra_eff");
	truefile += tag; gpfile += tag; varfile += tag; efffile += tag;
	std::ofstream trueOut(truefile.c_str(), std::ios::out),
	                gpOut(gpfile.c_str(),   std::ios::out),
                       varOut(varfile.c_str(),  std::ios::out),
                       effOut(efffile.c_str(),  std::ios::out);
	gpOut  << std::scientific; varOut << std::scientific;
	effOut << std::scientific; if (true_plot) trueOut << std::scientific;
	RealVector u_pt(2), x_pt(2);
	Real lbnd =  -5., ubnd =  5.;
	//Real lbnd = -10., ubnd = 10.;
	Real interval = (ubnd-lbnd)/100.;
	for (size_t i=0; i<101; i++){
	  u_pt[0] = lbnd + float(i)*interval;
	  for (size_t j=0; j<101; j++){
	    u_pt[1] = lbnd + float(j)*interval;
	    
	    uSpaceModel.continuous_variables(u_pt);
	    ActiveSet set = uSpaceModel.current_response().active_set();
	    set.request_values(0); set.request_value(1, respFnCount);
	    uSpaceModel.compute_response(set);
	    const Response& gp_resp = uSpaceModel.current_response();
	    const RealVector& gp_fn = gp_resp.function_values();
	    
	    gpOut << '\n' << setw(13) << u_pt[0] << ' ' << setw(13)
		  << u_pt[1] << ' ' << setw(13) << gp_fn[respFnCount];
	    
	    RealVector variance;
	    if (mppSearchType == EGRA_X) { // Recast( DataFit( iteratedModel ) )
	      // RecastModel::derived_compute_response() propagates u_pt to x_pt
	      Model& dfs_model = uSpaceModel.subordinate_model();
	      variance = dfs_model.approximation_variances(
		dfs_model.current_variables()); // x_pt
	    }
	    else // EGRA_U: DataFit( Recast( iteratedModel ) )
	      variance = uSpaceModel.approximation_variances(
		uSpaceModel.current_variables()); // u_pt
	    
	    varOut << '\n' << setw(13) << u_pt[0] << ' ' << setw(13)
		   << u_pt[1] << ' ' << setw(13) << variance[respFnCount];
	    
	    Real eff = expected_feasibility(gp_fn, u_pt);
	    
	    effOut << '\n' << setw(13) << u_pt[0] << ' ' << setw(13)
		   << u_pt[1] << ' ' << setw(13) << -eff;

	    // plotting the true function can be expensive, but is available
	    if (true_plot) {
	      uSpaceModel.component_parallel_mode(TRUTH_MODEL);
	      natafTransform.trans_U_to_X(u_pt,x_pt);
	      iteratedModel.continuous_variables(x_pt);
	      set = iteratedModel.current_response().active_set();
	      set.request_values(0); set.request_value(1, respFnCount);
	      iteratedModel.compute_response(set);
	      const Response& true_resp = iteratedModel.current_response();
	      const RealVector& true_fn = true_resp.function_values();
	      
	      trueOut << '\n' << setw(13) << u_pt[0] << ' ' << setw(13)
		      << u_pt[1] << ' ' << setw(13) << true_fn[respFnCount];
	    }
	  }
	  gpOut << std::endl; varOut << std::endl; effOut << std::endl;
	  if (true_plot) trueOut << std::endl;
	}
      }
#endif //DEBUG_PLOTS
    }
#endif //DEBUG

  } // end loop over response fns
}


void NonDGlobalReliability::importance_sampling()
{
  // rep needed for access to functions not mapped to Iterator level
  NonDAdaptImpSampling* importance_sampler_rep
    = (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
  // if the approximation is built in x-space, then importanceSampler must
  // perform inverse transformations on gp_inputs
  // if the approximation is build in u-space, only the cdfFlag is needed
  // to define which samples are failures
  if (mppSearchType == EGRA_X)
    importance_sampler_rep->initialize_random_variables(natafTransform);

  size_t i;
  statCount = 0;
  const ShortArray& final_res_asv = finalStatistics.active_set_request_vector();
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {

    // The most general case is to allow a combination of response, probability,
    // and reliability level specifications for each response function.
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           gl_len = requestedGenRelLevels[respFnCount].length(),
           num_levels = rl_len + pl_len + gl_len;

    RealVectorArray gp_inputs;
    if (num_levels==0) {
      uSpaceModel.component_parallel_mode(TRUTH_MODEL);
      // don't use derivatives in the importance sampling
      ActiveSet set = iteratedModel.current_response().active_set();
      set.request_values(0); set.request_value(1, respFnCount);
      iteratedModel.compute_response(set);
      const Response& true_resp = iteratedModel.current_response();
      const RealVector& true_fn = true_resp.function_values();
      finalStatistics.function_value(true_fn[respFnCount], statCount);
    }
    else {
      // extract the approximation data from the surrogate model.
      // Note 1: this data is either x-space (EGRA_X) or u-space (EGRA_U).
      // Note 2: this returns _all_ truth model data for this response fn,
      //         including initial DACE and EGRA added points for _all_ levels.
      // TO DO:  likely need to remove DACE and partition remaining data among
      //         levels for importance sampling efficiency.
      const Pecos::SurrogateData& gp_data
	= uSpaceModel.approximation_data(respFnCount);
      size_t num_data_pts = gp_data.size();
      gp_inputs.resize(num_data_pts);
      for (i=0; i<num_data_pts; ++i)
	gp_inputs[i] = gp_data.continuous_variables(i); // view OK
    }
    statCount++;

    // Standard deviations & sensitivities not available, skip
    statCount++;

    // Loop over response/probability/reliability levels
    for (levelCount=0; levelCount<num_levels; levelCount++) {

      Cout << "\n<<<<< Performing importance sampling for response function " 
	   << respFnCount+1 << " level " << levelCount+1 << '\n';

      bool ria_flag = (levelCount < rl_len) ? true : false;
      if (ria_flag) {
	Real z = computedRespLevels[respFnCount][levelCount] 
	       = requestedRespLevels[respFnCount][levelCount];
	importance_sampler_rep->initialize(gp_inputs, respFnCount, 0., z);
      }
      else
	importance_sampler_rep->initialize(gp_inputs, respFnCount, 0.,
	  computedRespLevels[respFnCount][levelCount]);

      // no summary output since on-the-fly constructed:
      importanceSampler.run_iterator(Cout);

      Real p = importance_sampler_rep->final_probability();
#ifdef DEBUG
      Cout << "\np = " << p << std::endl;
#endif // DEBUG
      // RIA z-bar -> p
      computedProbLevels[respFnCount][levelCount] = p;
      // RIA z-bar -> generalized beta
      Real gen_beta = -Pecos::Phi_inverse(p);
      computedGenRelLevels[respFnCount][levelCount] = gen_beta;
      switch (respLevelTarget) {
      case PROBABILITIES:
	finalStatistics.function_value(p, statCount++);        break;
      case GEN_RELIABILITIES:
	finalStatistics.function_value(gen_beta, statCount++); break;
      }
    }
  }
}


void NonDGlobalReliability::
EIF_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) {
    Real ei = nondGlobRelInstance->expected_improvement(
      sub_model_response.function_values(), recast_vars);
    recast_response.function_value(ei, 0);
  }
}


void NonDGlobalReliability::
EFF_objective_eval(const Variables& sub_model_vars,
		   const Variables& recast_vars,
		   const Response& sub_model_response,
		   Response& recast_response)
{
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1) {
    Real ef = nondGlobRelInstance->expected_feasibility(
      sub_model_response.function_values(), recast_vars);
    recast_response.function_value(ef, 0);
  }
}


Real NonDGlobalReliability::
expected_improvement(const RealVector& expected_values,
		     const Variables& recast_vars)
{
  // Get variance from the GP; Expected values are passed in
  // If GP built in x-space, transform input point to x-space to get variance
  RealVector variances;
  if (mppSearchType == EGRA_X) { // uSpaceModel = Recast(DataFit(iteratedModel))
    Model& dfs_model = uSpaceModel.subordinate_model();
    // assume recast_vars have been propagated to GP just prior to call
    variances
      = dfs_model.approximation_variances(dfs_model.current_variables());
  }
  else                   // EGRA_U: uSpaceModel = DataFit(Recast(iteratedModel))
    variances = uSpaceModel.approximation_variances(recast_vars);
    
  Real mean = expected_values[respFnCount];
  Real stdv = std::sqrt(variances[respFnCount]);

  // Calculate and apply penalty to the mean
  Real beta_star = 0.; // TO DO
  // calculate the equality constraint: beta* - bar-beta*
  Real cfn = beta_star - requestedTargetLevel;
  Real penalty = constraint_penalty(cfn, recast_vars.continuous_variables());
  // Calculation of EI will have different form for the CDF/CCDF cases
  Real penalized_mean = (pmaMaximizeG) ? mean - penalty : mean + penalty;
  
  // Calculate the expected improvement
  // Note: This is independent of the CDF/CCDF check

  Real ei, cdf, pdf;
  Real snv = (fnStar-penalized_mean); // not normalized yet
  if(std::fabs(snv)>=std::fabs(stdv)*50.0) {
    //this will trap the denominator=0.0 case even if numerator=0.0
    pdf = 0.;
    cdf = (snv > 0.) ? 1. : 0.;
  }
  else{
    snv /= stdv; // now snv is the standard normal variate
    cdf = Pecos::Phi(snv);
    pdf = Pecos::phi(snv);
  }
  ei = (pmaMaximizeG) ? (penalized_mean - fnStar)*(1.-cdf) + stdv*pdf
                      : (fnStar - penalized_mean)*cdf      + stdv*pdf;
  return -ei; // return -EI because we are maximizing EI
}


Real NonDGlobalReliability::
expected_feasibility(const RealVector& expected_values,
		     const Variables& recast_vars)
{
  // Get variance from the GP; Expected values are passed in
  // If GP built in x-space, transform input point to x-space to get variance
  RealVector variances;
  if (mppSearchType == EGRA_X) { // uSpaceModel = Recast(DataFit(iteratedModel))
    Model& dfs_model = uSpaceModel.subordinate_model();
    // assume recast_vars have been propagated to GP just prior to call
    variances
      = dfs_model.approximation_variances(dfs_model.current_variables());
  }
  else                   // EGRA_U: uSpaceModel = DataFit(Recast(iteratedModel))
    variances = uSpaceModel.approximation_variances(recast_vars);
  
  Real mean  = expected_values[respFnCount],
       stdv  = std::sqrt(variances[respFnCount]),
       zbar  = requestedTargetLevel,
       alpha = 2.; // may want to try values other than 2

  // calculate standard normal variate +/- alpha
  
  Real cdfz, pdfz, cdfp, pdfp, cdfm, pdfm;
  Real snvz = (zbar - mean);
  if (std::fabs(snvz) >= std::fabs(stdv)*50.) {
    pdfm = pdfp = pdfz = 0.;
    cdfm = cdfp = cdfz = (snvz > 0.) ? 1. : 0.;
  }
  else {
    snvz /= stdv;             pdfz = Pecos::phi(snvz); cdfz = Pecos::Phi(snvz);
    Real snvp = snvz + alpha; pdfp = Pecos::phi(snvp); cdfp = Pecos::Phi(snvp);
    Real snvm = snvz - alpha; pdfm = Pecos::phi(snvm); cdfm = Pecos::Phi(snvm);
  }
  // calculate expected feasibility function
  Real ef = (mean - zbar)*(2.*cdfz     -cdfm -cdfp)- //exploit
                     stdv*(2.*pdfz     -pdfm -pdfp - //explore
			   alpha*cdfp + alpha*cdfm);
  //Real ef = (mean - zbar)*(2.*Phi(snvz) - Phi(snvm) - Phi(snvp)) -  //exploit
  //                   stdv*(2.*phi(snvz) - phi(snvm) - phi(snvp)  -  //explore
  //   		              alpha*Phi(snvp) + alpha*Phi(snvm));

  return -ef;  // return -EF because we are maximizing
}


void NonDGlobalReliability::get_best_sample()
{
  // Pull the samples and responses from data used to build latest GP
  //   and apply any penalties to calculate fnStar for use in the 
  //   expected improvement function
  // This is only done for PMA - there is no "best solution" for
  //   the expected feasibility function used in RIA

  Iterator&             dace_iterator  = uSpaceModel.subordinate_iterator();
  const RealMatrix&     true_vars_x    = dace_iterator.all_samples();
  const IntResponseMap& true_responses = dace_iterator.all_responses();
  size_t i, j, num_samples = true_vars_x.numCols(),
    num_vars = true_vars_x.numRows();
  
  // If GP built in x-space, transform true_vars_x to u-space to calculate beta
  RealVectorArray true_c_vars_u(num_samples); RealVector true_vars_x_cv;
  for (i=0; i<num_samples; i++) {
    true_vars_x_cv = Teuchos::getCol(Teuchos::View,
      const_cast<RealMatrix&>(true_vars_x), (int)i);
    if (mppSearchType == EGRA_X)
      natafTransform.trans_X_to_U(true_vars_x_cv, true_c_vars_u[i]);
    else
      true_c_vars_u[i] = true_vars_x_cv; // view OK
  }

  // Calculation of fnStar will have different form for CDF/CCDF cases
  fnStar = DBL_MAX; IntRespMCIter it;
  for (i=0, it=true_responses.begin(); i<num_samples; i++, ++it) {
    // calculate the reliability index (beta)
    Real beta_star = 0., penalized_response; // TO DO
    // calculate the equality constraint: u'u - beta_target^2
    Real cfn = beta_star - requestedTargetLevel;
    Real penalty = constraint_penalty(cfn, true_c_vars_u[i]);
    penalized_response = (pmaMaximizeG) ?
      it->second.function_value(0) - penalty :
      it->second.function_value(0) + penalty;
    if ( ( pmaMaximizeG && penalized_response > fnStar) ||
	 (!pmaMaximizeG && penalized_response < fnStar) )
      fnStar = penalized_response;
  }
}


Real NonDGlobalReliability::
constraint_penalty(const Real& c_viol, const RealVector& u)
{
  if (meritFunctionType == PENALTY_MERIT)
    return exp(approxIters/10.)*c_viol*c_viol;  // try other schedules
  else if (meritFunctionType == AUGMENTED_LAGRANGIAN_MERIT) {
    if (lastIterateAccepted)
      augLagrangeMult += 2.*penaltyParameter*c_viol;
    else 
      penaltyParameter *= 2.;
    return augLagrangeMult*c_viol + penaltyParameter*c_viol*c_viol;
  }
  else if (meritFunctionType == LAGRANGIAN_MERIT) {
#ifdef DAKOTA_F90
    // form [A] = grad[u'u - beta^2]
    RealVector A(numContAleatUncVars, false);
    for (size_t i=0; i<numContAleatUncVars; i++)
      A[i] = 2.*u[i];

    // form -{grad_f} = m_grad_f = -grad[G_hat(u)]
    uSpaceModel.continuous_variables(u);
    uSpaceModel.compute_response();
    const Real* grad_f = uSpaceModel.current_response().function_gradient(0);
    RealVector m_grad_f(numContAleatUncVars, false);
    for (size_t i=0; i<numContAleatUncVars; ++i)
      m_grad_f[i] = -grad_f[i];

    // solve for lambda : [A]{lambda} = {m_grad_f}
    int ierr, nsetp, m = numContAleatUncVars, n = 1;
    Real res_norm;
    IntVector index(1);
    RealVector lambda(1), w(1), bnd(2);
    bnd[0] = -DBL_MAX; bnd[1] = DBL_MAX;
    BVLS_WRAPPER_FC(A.values(), m, n, m_grad_f.values(), bnd.values(),
		    lambda.values(), res_norm, nsetp, w.values(),
		    index.values(), ierr);
    if (ierr) {
      Cerr << "\nError: BVLS failed in constraint_penalty() in NonDGR"
	   << std::endl;
      abort_handler(-1);
    }

    lagrangeMult = lambda[0];
    return lagrangeMult*c_viol;
#endif // DAKOTA_F90
  }
  else
    return 0.; // add NO_PENALTY to the enum instead?
}


void NonDGlobalReliability::print_results(std::ostream& s)
{
  size_t i, j;
  const StringArray& fn_labels = iteratedModel.response_labels();
  s << "-----------------------------------------------------------------\n";

  for (i=0; i<numFunctions; i++) {

    // output CDF/CCDF response/probability pairs
    size_t num_levels = computedRespLevels[i].length();
    if (num_levels) {
      if (cdfFlag)
        s << "Cumulative Distribution Function (CDF) for ";
      else
        s << "Complementary Cumulative Distribution Function (CCDF) for ";
      s << fn_labels[i] << ":\n     Response Level  Probability Level  "
	<< "Reliability Index  General Rel Index\n     --------------  "
	<< "-----------------  -----------------  -----------------\n";
      for (j=0; j<num_levels; j++)
        s << "  " << setw(write_precision+7) << computedRespLevels[i][j]
	  << "  " << setw(write_precision+7) << computedProbLevels[i][j]
	  << setw(2*write_precision+18) << computedGenRelLevels[i][j]  << '\n';
    }
  }

  //s << "Final statistics:\n" << finalStatistics;

  s << "-----------------------------------------------------------------"
    << std::endl;
}

} // namespace Dakota
