/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "RecastModel.hpp"
#include "DataTransformModel.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDLHSSampling.hpp"
#include "NPSOLOptimizer.hpp"
#include "SNLLOptimizer.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

#include "LHSDriver.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

// initialization of statics
NonDBayesCalibration* NonDBayesCalibration::nonDBayesInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::
NonDBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDCalibration(problem_db, model),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  chainSamples(0), chainCycles(1),
  randomSeed(probDescDB.get_int("method.random_seed")),
  obsErrorMultiplierMode(
    probDescDB.get_ushort("method.nond.calibrate_error_mode")),
  numHyperparams(0),
  invGammaAlphas(probDescDB.get_rv("method.nond.hyperprior_alphas")),
  invGammaBetas(probDescDB.get_rv("method.nond.hyperprior_betas")),
  adaptPosteriorRefine(
    probDescDB.get_bool("method.nond.adaptive_posterior_refinement")),
  proposalCovarType(
    probDescDB.get_string("method.nond.proposal_covariance_type")),
  proposalCovarData(probDescDB.get_rv("method.nond.proposal_covariance_data")),
  proposalCovarFilename(
    probDescDB.get_string("method.nond.proposal_covariance_filename")),
  proposalCovarInputType(
    probDescDB.get_string("method.nond.proposal_covariance_input_type")),
  burnInSamples(probDescDB.get_int("method.burn_in_samples")),
  posteriorStatsKL(probDescDB.get_bool("method.posterior_stats.kl_divergence")),
  posteriorStatsMutual(probDescDB.get_bool("method.posterior_stats.mutual_info")),
  subSamplingPeriod(probDescDB.get_int("method.sub_sampling_period")),
  exportMCMCFilename(
    probDescDB.get_string("method.nond.export_mcmc_points_file")),
  exportMCMCFormat(probDescDB.get_ushort("method.nond.export_mcmc_format"))
{
  // assign default proposalCovarType
  if (proposalCovarType.empty()) {
    if (emulatorType) proposalCovarType = "derivatives"; // misfit Hessian
    else              proposalCovarType = "prior";       // prior covariance
  }

  // manage sample partitions and defaults
  int samples_spec = probDescDB.get_int("method.nond.chain_samples");
  if (proposalCovarType == "derivatives") {
    int pc_update_spec
      = probDescDB.get_int("method.nond.proposal_covariance_updates");
    if (pc_update_spec < 1) { // default partition: update every 100 samples
      chainSamples  = 100;
      chainCycles = (int)floor((Real)samples_spec / (Real)chainSamples + .5);
    }
    else { // partition as specified
      chainSamples  = (int)floor((Real)samples_spec / (Real)pc_update_spec + .5);
      chainCycles = pc_update_spec;
    }
  }
  else
    { chainSamples = samples_spec; chainCycles = 1; }

  if (randomSeed != 0)
  {
    Cout << " NonDBayes Seed (user-specified) = " << randomSeed << std::endl;
  }
  else {
    // Use NonD convenience function for system seed
    randomSeed = generate_system_seed();
    Cout << " NonDBayes Seed (system-generated) = " << randomSeed << std::endl;
  }

  // assign default maxIterations (DataMethod default is -1)
  if (adaptPosteriorRefine && maxIterations < 0)
    maxIterations = 25;

  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    standardizedSpace = true; break; // natafTransform defined w/i NonDExpansion
  default:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");

    // define local natafTransform, whether standardized space or not,
    // since we utilize x-space bounds, moments, density routines
    initialize_random_variable_transformation();
    initialize_random_variable_types(ASKEY_U); // need ranVarTypesX below
    // initialize_random_variable_parameters() is performed at run time
    initialize_random_variable_correlations();
    //initialize_final_statistics(); // statistics set is not default

    // only needed if Nataf transform will actually be performed
    if (standardizedSpace)
      verify_correlation_support(ASKEY_U);
    break;
  }

  // --------------------------------------------------
  // Construct mcmcModel (no emulation, GP, PCE, or SC)
  // for use in likelihood evaluations
  // --------------------------------------------------
  short mcmc_deriv_order = 1;
  switch (emulatorType) {

  case PCE_EMULATOR: case SC_EMULATOR: {
    const UShortArray& level_seq
      = probDescDB.get_usa("method.nond.sparse_grid_level");
    const RealVector& dim_pref
      = probDescDB.get_rv("method.nond.dimension_preference");    // not exposed
    bool derivs = probDescDB.get_bool("method.derivative_usage"); // not exposed
    NonDExpansion* se_rep;
    if (emulatorType == SC_EMULATOR) { // SC sparse grid interpolation
      se_rep = new NonDStochCollocation(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	false, derivs);
      mcmc_deriv_order = 3; // Hessian computations not yet implemented for SC
    }
    else {
      if (!level_seq.empty()) // PCE with spectral projection via sparse grid
	se_rep = new NonDPolynomialChaos(iteratedModel,
	  Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	  false, false);
      else { 
        // regression PCE: LeastSq/CS (exp_order,colloc_ratio), OLI (colloc_pts)
	const UShortArray& exp_order_seq
	  = probDescDB.get_usa("method.nond.expansion_order");
	short exp_coeffs_approach = (exp_order_seq.empty()) ?
	  Pecos::ORTHOG_LEAST_INTERPOLATION : Pecos::DEFAULT_REGRESSION;
	se_rep = new NonDPolynomialChaos(iteratedModel,
	  exp_coeffs_approach, exp_order_seq, dim_pref,
	  probDescDB.get_sza("method.nond.collocation_points"), // pts sequence
	  probDescDB.get_real("method.nond.collocation_ratio"), // single scalar
	  randomSeed, EXTENDED_U, false, derivs,	
	  probDescDB.get_bool("method.nond.cross_validation"), // not exposed
	  probDescDB.get_string("method.import_build_points_file"),
	  probDescDB.get_ushort("method.import_build_format"),
	  probDescDB.get_bool("method.import_build_active_only"));
      }
      mcmc_deriv_order = 7; // Hessian computations implemented for PCE
    }
    stochExpIterator.assign_rep(se_rep);
    // no CDF or PDF level mappings
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
      empty_rv_array, respLevelTarget, respLevelTargetReduce, cdfFlag, false);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    mcmcModel = stochExpIterator.algorithm_space_model(); // shared rep
    natafTransform = se_rep->variable_transformation();   // shared rep
    break;
  }

  case GP_EMULATOR: case KRIGING_EMULATOR: {
    String sample_reuse; String approx_type;
    if (emulatorType == GP_EMULATOR)
      { approx_type = "global_gaussian"; mcmc_deriv_order = 3; } // grad support
    else
      { approx_type = "global_kriging";  mcmc_deriv_order = 7; } // grad,Hess
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
    if (probDescDB.get_bool("method.derivative_usage")) {
      // derivatives for emulator construction (not emulator evaluation)
      if (iteratedModel.gradient_type() != "none") data_order |= 2;
      if (iteratedModel.hessian_type()  != "none") data_order |= 4;
    }
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    int samples = probDescDB.get_int("method.build_samples");
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_build_points_file");
    if (!import_pts_file.empty())
      { samples = 0; sample_reuse = "all"; }
     
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator; Model lhs_model;
    // NKM requires finite bounds for scaling and init of correlation lengths.
    // Default truncation is +/-10 sigma, which may be overly conservative for
    // these purposes, but +/-3 sigma has little to no effect in current tests.
    bool truncate_bnds = (emulatorType == KRIGING_EMULATOR);
    if (standardizedSpace)
      transform_model(iteratedModel, lhs_model, truncate_bnds);//, 3.);
    else
      lhs_model = iteratedModel; // shared rep
    // Unlike EGO-based approaches, use ACTIVE sampling mode to concentrate
    // samples in regions of higher prior density
    NonDLHSSampling* lhs_rep = new
      NonDLHSSampling(lhs_model, sample_type, samples, randomSeed,
        probDescDB.get_string("method.random_number_generator"));
    lhs_iterator.assign_rep(lhs_rep, false);

    // natafTransform is not fully updated at this point, but using
    // a shallow copy allows run time updates to propagate
    if (standardizedSpace)
      lhs_rep->initialize_random_variables(natafTransform); // shallow copy

    ActiveSet gp_set = lhs_model.current_response().active_set(); // copy
    gp_set.request_values(mcmc_deriv_order); // for misfit Hessian
    mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, lhs_model,
      gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
      outputLevel, sample_reuse, import_pts_file,
      probDescDB.get_ushort("method.import_build_format"),
      probDescDB.get_bool("method.import_build_active_only")), false);
    break;
  }

  case NO_EMULATOR:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
    if (standardizedSpace) transform_model(iteratedModel, mcmcModel);//dist bnds
    else                   mcmcModel = iteratedModel; // shared rep

    if (mcmcModel.gradient_type() != "none") mcmc_deriv_order |= 2;
    if (mcmcModel.hessian_type()  != "none") mcmc_deriv_order |= 4;
    break;
  }

  // Initialize sizing for hyperparameters (observation error), not
  // currently part of a RecastModel
  size_t num_resp_groups = 
    mcmcModel.current_response().shared_data().num_response_groups(); 
  if (obsErrorMultiplierMode == CALIBRATE_ONE)
    numHyperparams = 1;
  else if (obsErrorMultiplierMode == CALIBRATE_PER_EXPER)
    numHyperparams = numExperiments;
  else if (obsErrorMultiplierMode == CALIBRATE_PER_RESP)
    numHyperparams = num_resp_groups;
  else if (obsErrorMultiplierMode == CALIBRATE_BOTH)
    numHyperparams = numExperiments * num_resp_groups;

  // Setup priors distributions on hyper-parameters
  if ( (invGammaAlphas.length()  > 1 &&
	invGammaAlphas.length() != numHyperparams) ||
       (invGammaAlphas.length() != invGammaBetas.length()) ) {
    Cerr << "\nError: hyperprior_alphas and hyperprior_betas must both have "
         << "length 1 or number of calibrated\n       error multipliers.\n";
    abort_handler(PARSE_ERROR);
  }
  invGammaDists.resize(numHyperparams);
  for (size_t i=0; i<numHyperparams; ++i) {
    // alpha = (mean/std_dev)^2 + 2
    // beta = mean*(alpha-1)
    // default:
    Real alpha = 102.0, beta = 103.0;  
    // however chosen to have mode = beta/(alpha+1) = 1.0 (initial point)
    //   mean = beta/(alpha-1) ~ 1.0
    //   s.d. ~ 0.1
    if (invGammaAlphas.length() == 1)
      { alpha = invGammaAlphas[0]; beta = invGammaBetas[0]; }
    else if (invGammaAlphas.length() == numHyperparams)
      { alpha = invGammaAlphas[i]; beta = invGammaBetas[i]; }
    // BMA TODO: could store only one inverse gamma if all parameters the same
    invGammaDists[i] = Pecos::RandomVariable(Pecos::INV_GAMMA);
    Pecos::InvGammaRandomVariable* rv_rep = 
      (Pecos::InvGammaRandomVariable*)(invGammaDists[i].random_variable_rep());
    rv_rep->update(alpha, beta);
  }

  // Now the underlying simulation model mcmcModel is setup; wrap it
  // in a data transformation, making sure to allocate gradient/Hessian space
  if (calibrationData) {
    residualModel.assign_rep
      (new DataTransformModel(mcmcModel, expData, numHyperparams, 
                              obsErrorMultiplierMode, mcmc_deriv_order), false);
    // update bounds for hyper-parameters
    Real dbl_inf = std::numeric_limits<Real>::infinity();
    for (size_t i=0; i<numHyperparams; ++i) {
      residualModel.continuous_lower_bound(0.0,     numContinuousVars + i);
      residualModel.continuous_upper_bound(dbl_inf, numContinuousVars + i);
    }
  }
  else
    residualModel = mcmcModel;  // shallow copy

  // -------------------------------------
  // Construct optimizer for MAP pre-solve
  // -------------------------------------
  // Emulator:     on by default; can be overridden with "pre_solve none"
  // No emulator: off by default; can be activated  with "pre_solve {sqp,nip}"
  //              relies on mapOptimizer ctor to enforce min derivative support
  unsigned short opt_alg_override
    = probDescDB.get_ushort("method.nond.pre_solve_method");
  if ( opt_alg_override == SUBMETHOD_SQP || opt_alg_override == SUBMETHOD_NIP ||
       ( emulatorType && opt_alg_override != SUBMETHOD_NONE ) ) {

    Sizet2DArray vars_map_indices, primary_resp_map_indices(1),
      secondary_resp_map_indices;
    primary_resp_map_indices[0].resize(numTotalCalibTerms);
    for (size_t i=0; i<numTotalCalibTerms; ++i)
      primary_resp_map_indices[0][i] = i;
    bool nonlinear_vars_map = false; BoolDequeArray nonlinear_resp_map(1);
    nonlinear_resp_map[0] = BoolDeque(numTotalCalibTerms, true);
    SizetArray recast_vc_totals;  // empty: no change in size
    BitArray all_relax_di, all_relax_dr; // empty: no discrete relaxation

    switch (opt_alg_override) {
    case SUBMETHOD_SQP:
#ifndef HAVE_NPSOL
      Cerr << "\nWarning: this executable not configured with NPSOL SQP."
	   << "\n         MAP pre-solve not available." << std::endl;
      opt_alg_override = SUBMETHOD_DEFAULT; // model,optimizer not constructed
#endif
      break;
    case SUBMETHOD_NIP:
#ifndef HAVE_OPTPP
      Cerr << "\nWarning: this executable not configured with OPT++ NIP."
	   << "\n         MAP pre-solve not available." << std::endl;
      opt_alg_override = SUBMETHOD_DEFAULT; // model,optimizer not constructed
#endif
      break;
    case SUBMETHOD_DEFAULT: // use full Newton, if available
#ifdef HAVE_OPTPP
      opt_alg_override = SUBMETHOD_NIP;
#elif HAVE_NPSOL
      opt_alg_override = SUBMETHOD_SQP;
#else
      Cerr << "\nWarning: this executable not configured with NPSOL or OPT++."
	   << "\n         MAP pre-solve not available." << std::endl;
#endif
      break;
    }

    // recast ActiveSet requests if full-Newton NIP with gauss-Newton misfit
    // (avoids error in unsupported Hessian requests in Model::manage_asv())
    short nlp_resp_order = 3; // quasi-Newton optimization
    void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
    if (opt_alg_override == SUBMETHOD_NIP) {
      nlp_resp_order = 7; // size RecastModel response for full Newton Hessian
      if (mcmc_deriv_order == 3) // map asrv for Gauss-Newton approx
        set_recast = gnewton_set_recast;
    }

    // RecastModel for bound-constrained argmin(misfit - log prior)
    if (opt_alg_override)
      negLogPostModel.assign_rep(new 
	RecastModel(residualModel, vars_map_indices, recast_vc_totals, 
		    all_relax_di, all_relax_dr, nonlinear_vars_map, NULL,
		    set_recast, primary_resp_map_indices, 
		    secondary_resp_map_indices, 0, nlp_resp_order, 
		    nonlinear_resp_map, neg_log_post_resp_mapping, NULL),false);

    switch (opt_alg_override) {
#ifdef HAVE_NPSOL
    case SUBMETHOD_SQP: {
      // SQP with BFGS Hessians
      int npsol_deriv_level = 3;
      mapOptimizer.assign_rep(new
	NPSOLOptimizer(negLogPostModel, npsol_deriv_level, convergenceTol),
	false);
      break;
    }
#endif
#ifdef HAVE_OPTPP
    case SUBMETHOD_NIP:
      // full Newton (OPTPP::OptBCNewton)
      mapOptimizer.assign_rep(new 
	SNLLOptimizer("optpp_newton", negLogPostModel), false);
      break;
#endif
    }
  }

  int mcmc_concurrency = 1; // prior to concurrent chains
  maxEvalConcurrency *= mcmc_concurrency;
}


NonDBayesCalibration::~NonDBayesCalibration()
{ }


void NonDBayesCalibration::derived_init_communicators(ParLevLIter pl_iter)
{
  //iteratedModel.init_communicators(maxEvalConcurrency);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.init_communicators(pl_iter);              break;
  default:
    mcmcModel.init_communicators(pl_iter, maxEvalConcurrency); break;
  }
}


void NonDBayesCalibration::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  //iteratedModel.set_communicators(maxEvalConcurrency);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.set_communicators(pl_iter);              break;
  default:
    mcmcModel.set_communicators(pl_iter, maxEvalConcurrency); break;
  }
}


void NonDBayesCalibration::derived_free_communicators(ParLevLIter pl_iter)
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.free_communicators(pl_iter);              break;
  default:
    mcmcModel.free_communicators(pl_iter, maxEvalConcurrency); break;
  }

  //iteratedModel.free_communicators(maxEvalConcurrency);
}


void NonDBayesCalibration::initialize_model()
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    stochExpIterator.run(pl_iter); break;
  }
  default: // GPs and NO_EMULATOR
    initialize_random_variable_parameters(); // standardizedSpace or not
    //initialize_final_statistics_gradients(); // not required
    if (standardizedSpace) transform_correlations();
    if (emulatorType)      mcmcModel.build_approximation();
    break;
  }
  Cout << "KL Divergence Flag " << posteriorStatsKL << '\n';
  Cout << "Mutual Info Flag " << posteriorStatsMutual << '\n';
}


/** Calculate the log-likelihood, accounting for contributions from
    covariance and hyperparameters, as well as constant term:

      log(L) = -1/2*Nr*log(2*pi) - 1/2*log(det(Cov)) - 1/2*r'(Cov^{-1})*r

    The passed residual_resp must already be size-adjusted, differenced with any
    data, if present, and scaled by covariance^{-1/2}. */
Real NonDBayesCalibration::
log_likelihood(const RealVector& residuals, const RealVector& all_params)
{
  // if needed, extract the trailing hyper-parameters
  RealVector hyper_params;
  if (numHyperparams > 0)
    hyper_params = RealVector(Teuchos::View, 
                              all_params.values() + numContinuousVars, 
                              numHyperparams);

  Real half_nr_log2pi = numTotalCalibTerms * HALF_LOG_2PI;
  Real half_log_det = 
    expData.half_log_cov_determinant(hyper_params, obsErrorMultiplierMode);

  // misfit defined as 1/2 r^T (mult^2*Gamma_d)^{-1} r
  Real misfit = residuals.dot( residuals ) / 2.0;

  Real log_like = -half_nr_log2pi - half_log_det - misfit;

  return log_like;
}


void NonDBayesCalibration::prior_cholesky_factorization()
{
  // factorization to be performed offline (init time) and used online
  int i, j, num_params = numContinuousVars + numHyperparams;
  priorCovCholFactor.shape(num_params, num_params); // init to 0

  if (!standardizedSpace && natafTransform.x_correlation()) {
    Teuchos::SerialSpdDenseSolver<int, Real> corr_solver;
    RealSymMatrix prior_cov_matrix;//= ();

    Cerr << "prior_cholesky_factorization() not yet implmented for this case."
	 << std::endl;
    abort_handler(-1);

    corr_solver.setMatrix( Teuchos::rcp(&prior_cov_matrix, false) );
    corr_solver.factor(); // Cholesky factorization (LL^T) in place
    // assign lower triangle
    for (i=0; i<num_params; ++i)
      for (j=0; j<=i; ++j)
	priorCovCholFactor(i, j) = prior_cov_matrix(i, j);
  }
  else {
    RealRealPairArray dist_moments = (standardizedSpace) ?
      natafTransform.u_moments() : natafTransform.x_moments();
    for (i=0; i<numContinuousVars; ++i)
      priorCovCholFactor(i,i) = dist_moments[i].second;
    // for now we assume a variance when the inv gamma has infinite moments
    for (i=0; i<numHyperparams; ++i)
      if (invGammaDists[i].parameter(Pecos::IGA_ALPHA) > 2.0)
        priorCovCholFactor(numContinuousVars + i, numContinuousVars + i) = 
          invGammaDists[i].standard_deviation();
      else
        priorCovCholFactor(numContinuousVars + i, numContinuousVars + i) =
          0.05*(invGammaDists[i].mode());
  }
}


void NonDBayesCalibration::
get_positive_definite_covariance_from_hessian(const RealSymMatrix &hessian,
					      const RealMatrix& prior_chol_fact,
					      RealSymMatrix &covariance,
					      short output_lev)
{
  // precondition H_misfit by computing L^T H_misfit L where L is the Cholesky
  // factor of the prior covariance.  Important notes:
  // > in other contexts, we compute the Hessian of the negative log posterior
  //   from the Hessian of the misfit minus the Hessian of log prior density.
  // > In the context of defining a MVN covariance, we use the fact that the
  //   Hessian of the negative log of a normal density is 1 / variance, such
  //   that the inverse Hessian is simply the prior covariance.
  //   >> Thus, we justify use of the prior covariance, even for cases (e.g.,
  //      uniform, exp) where the Hessian of the log prior density is zero.
  // > For uncorrelated priors, L is simply diag[sigma_i].

  // Option 1: if augmenting with Hessian of negative log prior
  //           Hess of neg log posterior = Hess of misfit - Hess of log prior
  //const RealVector& c_vars = mcmcModel.continuous_variables();
  //augment_hessian_with_log_prior(log_hess, c_vars);

  // Option 2: if preconditioning with prior covariance using L^T H L
  // can use incoming Hessian as both input and output since an internal
  // temporary is created for H L prior to creating symmetric L^T H L
  int num_rows = hessian.numRows();
  RealSymMatrix LT_H_L(num_rows, false); // copy
  Teuchos::symMatTripleProduct(Teuchos::TRANS, 1., hessian,
			       prior_chol_fact, LT_H_L);

  // Compute eigenvalue decomposition of matrix A=QDQ'
  // In general, the eigenvalue decomposition of a matrix is A=QDinv(Q).
  // For real symmetric matrices A = QDQ', i.e. inv(Q) = Q'
  RealVector eigenvalues; RealMatrix eigenvectors;
  symmetric_eigenvalue_decomposition( LT_H_L, eigenvalues, eigenvectors );

  // Find smallest positive eigenvalue
  //Real min_eigval = std::numeric_limits<double>::max();
  //for ( int i=0; i<num_rows; i++)
  //  if ( eigenvalues[i] > 0. )
  //    min_eigval = std::min( eigenvalues[i], min_eigval );

#ifdef DEBUG
  Cout << "eigenvalues from symmetric_eigenvalue_decomposition:\n";
  write_data(Cout, eigenvalues);
#endif

  /*
  // Ensure hessian is positive definite by setting all negative eigenvalues 
  // to be positive.
  Real eigenval_tol = 1.e-4; // TO DO: tie to prior bounds?
  int i, j, num_neglect = 0;
  for (i=0; i<num_rows; ++i)
    if ( eigenvalues[i] < eigenval_tol )
      { eigenvalues[i] = eigenval_tol; ++num_neglect; }
    else
      break;

  // The covariance matrix is the inverse of the hessian so scale eigenvectors
  // by Q*inv(D)
  RealMatrix scaled_eigenvectors(num_rows, num_rows, false); // don't init to 0
  for ( i=0; i<num_rows; ++i ) {
    for ( j=0; j<num_neglect; ++j )
      scaled_eigenvectors(i,j) = 0.;
    for ( j=num_neglect; j<num_rows; ++j )
      scaled_eigenvectors(i,j) = eigenvectors(i,j) / eigenvalues[j];
  }
  covariance.shapeUninitialized( num_rows );
  covariance.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 
		       1., scaled_eigenvectors, eigenvectors, 0. );
  */

  // Form V and D
  Real eigenval_tol = 0.; //1.e-4; // Petra2014 suggests tol=1 in Fig 5.2
  int n, r, num_neglect = 0;
  for (n=0; n<num_rows; ++n)
    if ( eigenvalues[n] <= eigenval_tol ) ++num_neglect;
    else                                  break;
  int num_low_rank = num_rows - num_neglect, offset_r;
  RealSymMatrix D(num_low_rank); // init to 0;    r x r diagonal matrix
  RealMatrix V(num_rows, num_low_rank, false); // n x r matrix for r retained
  for (r=0; r<num_low_rank; ++r) {
    offset_r = r + num_neglect;
    Real lambda = eigenvalues[offset_r];
    D(r,r) = lambda / (lambda + 1.); // Sherman-Morrison-Woodbury
    for (n=0; n<num_rows; ++n)
      V(n,r) = eigenvectors(n,offset_r); // copy column
  }

  // Form I - V D V^T
  covariance.shapeUninitialized(num_rows);
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, -1., D, V, covariance);
  for (n=0; n<num_rows; ++n)
    covariance(n,n) += 1.;

  // form inv(hessian) = L (I - V D V^T) L^T
  // can use covariance as both input and output (see above)
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, 1., covariance,
			       prior_chol_fact, covariance);

  if (output_lev > NORMAL_OUTPUT) {
    Cout << "Hessian of negative log-likelihood (from misfit):\n";
    write_data(Cout, hessian, true, true, true);
    //Cout << "2x2 determinant = " << hessian(0,0) * hessian(1,1) -
    //  hessian(0,1) * hessian(1,0) << '\n';

    //Cout << "Cholesky factor of prior covariance:\n";
    //write_data(Cout, prior_chol_fact, true, true, true);

    Cout << "Prior-preconditioned misfit Hessian:\n";
    write_data(Cout, LT_H_L, true, true, true);
    //Cout << "2x2 determinant = " << LT_H_L(0,0) * LT_H_L(1,1) -
    //  LT_H_L(0,1) * LT_H_L(1,0) << '\n';
    if (num_neglect)
      Cout << "Hessian decomposition neglects " << num_neglect
           << " eigenvalues based on " << eigenval_tol << " tolerance.\n";
  }
  if (output_lev >= NORMAL_OUTPUT) {
    Cout << "Positive definite covariance from inverse of Hessian:\n";
    write_data(Cout, covariance, true, true, true);
    //Cout << "2x2 determinant = " << covariance(0,0) * covariance(1,1) -
    //  covariance(0,1) * covariance(1,0) << '\n';
  }

  //return num_neglect;
}


/** Response mapping callback used within RecastModel for MAP
    pre-solve. Computes 

      -log(post) = -log(like) - log(prior); where
      -log(like) = 1/2*Nr*log(2*pi) + 1/2*log(det(Cov)) + 1/2*r'(Cov^{-1})*r
                 = 1/2*Nr*log(2*pi) + 1/2*log(det(Cov)) + misfit

    (misfit defined as 1/2 r^T (mult^2*Gamma_d)^{-1} r) The passed
    residual_resp has been differenced, interpolated, and
    covariance-scaled */
void NonDBayesCalibration::
neg_log_post_resp_mapping(const Variables& residual_vars,
                          const Variables& nlpost_vars,
                          const Response& residual_resp,
                          Response& nlpost_resp)
{
  const RealVector& c_vars = nlpost_vars.continuous_variables();
  short nlpost_req = nlpost_resp.active_set_request_vector()[0];
  bool output_flag = (nonDBayesInstance->outputLevel >= DEBUG_OUTPUT);
  // if needed, extract the trailing hyper-parameters
  RealVector hyper_params;
  if (nonDBayesInstance->numHyperparams > 0)
    hyper_params = 
      RealVector(Teuchos::View, 
		 c_vars.values() + nonDBayesInstance->numContinuousVars, 
		 nonDBayesInstance->numHyperparams);

  if (nlpost_req & 1) {
    const RealVector& residuals = residual_resp.function_values();
    Real nlp = -nonDBayesInstance->log_likelihood(residuals, c_vars) - 
      nonDBayesInstance->log_prior_density(c_vars);
    nlpost_resp.function_value(nlp, 0);
    if (output_flag)
      Cout << "MAP pre-solve: negative log posterior = " << nlp << std::endl;
  }

  if (nlpost_req & 2) {
    // avoid copy by updating gradient vector in place
    RealVector log_grad = nlpost_resp.function_gradient_view(0);
    // Gradient contribution from misfit
    nonDBayesInstance->
      expData.build_gradient_of_sum_square_residuals(residual_resp, log_grad);
    // Add the contribution from 1/2*log(det(Cov))
    nonDBayesInstance->expData.half_log_cov_det_gradient
      (hyper_params, nonDBayesInstance->obsErrorMultiplierMode, 
       nonDBayesInstance->numContinuousVars, log_grad);
    // Add the contribution from -log(prior)
    nonDBayesInstance->augment_gradient_with_log_prior(log_grad, c_vars);
    if (output_flag) {
      Cout << "MAP pre-solve: negative log posterior gradient:\n";
      write_data(Cout, log_grad);
    }
  }

  if (nlpost_req & 4) {
    // avoid copy by updating Hessian matrix in place
    RealSymMatrix log_hess = nlpost_resp.function_hessian_view(0);
    // Hessian contribution from misfit
    nonDBayesInstance->
      expData.build_hessian_of_sum_square_residuals(residual_resp, log_hess);
    // Add the contribution from 1/2*log(det(Cov))
    nonDBayesInstance->expData.half_log_cov_det_hessian
      (hyper_params, nonDBayesInstance->obsErrorMultiplierMode, 
       nonDBayesInstance->numContinuousVars, log_hess);
    // Add the contribution from -log(prior)
    nonDBayesInstance->augment_hessian_with_log_prior(log_hess, c_vars);
    if (output_flag) {
      Cout << "MAP pre-solve: negative log posterior Hessian:\n";
      write_data(Cout, log_hess);
    }
  }

  //Cout << "nlpost_resp:\n" << nlpost_resp;
}

void NonDBayesCalibration::compute_statistics()
{
  // mcmcchain is either acceptanceChain or filtered_chain
  // mcmcfnvals is either acceptedFnVals or filteredFnVals
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_samples = acceptanceChain.numCols();
  int num_filtered = int((num_samples-burnin)/num_skip);
  size_t num_exp = expData.num_experiments();
  if (burnInSamples > 0 || subSamplingPeriod > 0)
  {
    RealMatrix filtered_chain;
    filtered_chain.shapeUninitialized(acceptanceChain.numRows(), num_filtered);
    filter_chain(acceptanceChain, filtered_chain);
    filteredFnVals.shapeUninitialized(acceptedFnVals.numRows(), num_filtered);
    filter_fnvals(acceptedFnVals, filteredFnVals);
    NonDSampling::compute_moments(filtered_chain, chainStats);
    NonDSampling::compute_moments(filteredFnVals, fnStats);
    // Print tabular file for filtered chain
    print_filtered_tabular(filtered_chain, filteredFnVals, predVals, 
      			   num_filtered, num_exp);
  }
  else
  {
    NonDSampling::compute_moments(acceptanceChain, chainStats);
    NonDSampling::compute_moments(acceptedFnVals, fnStats);
  }
  
  if (outputLevel >= NORMAL_OUTPUT) {
    compute_intervals(acceptanceChain, acceptedFnVals);
  }
}

void NonDBayesCalibration::filter_chain(RealMatrix& acceptance_chain, 
					RealMatrix& filtered_chain)
{
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int num_samples = acceptance_chain.numCols();
  int num_filtered = int((num_samples - burnin)/num_skip);
  int j = 0;
  for (int i = burnin; i < num_samples; ++i){
    if (i % num_skip == 0){
      const RealVector& param_vec = Teuchos::getCol(Teuchos::View, 
	  			    acceptance_chain, i);
      Teuchos::setCol(param_vec, j, filtered_chain);
      j++;
    }
  }
}

void NonDBayesCalibration::filter_fnvals(RealMatrix& accepted_fn_vals, 
    					 RealMatrix& filtered_fn_vals)
{
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int num_samples = accepted_fn_vals.numCols();
  int num_filtered = int((num_samples - burnin)/num_skip);
  int j = 0;
  for (int i = burnin; i < num_samples; ++i){
    if (i % num_skip == 0){
      const RealVector& col_vec = Teuchos::getCol(Teuchos::View, 
	  			  accepted_fn_vals, i);
      Teuchos::setCol(col_vec, j, filtered_fn_vals);
      j++;
    }
  }
}

void NonDBayesCalibration::compute_intervals(RealMatrix& acceptance_chain,
					     RealMatrix& accepted_fn_vals)
{
  std::ofstream interval_stream("dakota_mcmc_CredPredIntervals.dat");
  std::ostream& screen_stream = Cout;

  // Filter mcmc chain and corresponding function values
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_params = acceptance_chain.numRows();
  int num_samples = acceptance_chain.numCols();
  int num_filtered = int((num_samples-burnin)/num_skip);
  //RealMatrix filtered_fn_vals;
  RealMatrix filtered_chain;
  filteredFnVals.shapeUninitialized(numFunctions, num_filtered);
  filtered_chain.shapeUninitialized(acceptance_chain.numRows(), num_filtered);
  filter_chain(acceptance_chain, filtered_chain);
  filter_fnvals(accepted_fn_vals, filteredFnVals);
  // Make accepted function values the rows instead of the columns
  RealMatrix filtered_fn_vals_transpose(filteredFnVals, Teuchos::TRANS);
  // Augment function values with experimental uncertainty for prediction ints
  size_t num_exp = expData.num_experiments();
  size_t num_concatenated = num_exp*num_filtered;
  //RealMatrix predVals;
  predVals.shapeUninitialized(numFunctions, num_concatenated);
  compute_prediction_vals(filteredFnVals, predVals, 
      			  num_filtered, num_exp, num_concatenated);
  RealMatrix predVals_transpose(predVals, Teuchos::TRANS);

  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  size_t width = write_precision+7;
  
  // Calculate +/- 2sigma credibility intervals
  RealVector Fn_ave(numFunctions), Fn_stdevs(numFunctions),
	     Cred_interval_minima(numFunctions), 
	     Cred_interval_maxima(numFunctions);
  compute_col_means(filtered_fn_vals_transpose, Fn_ave); 
  compute_col_stdevs(filtered_fn_vals_transpose, Fn_ave, Fn_stdevs);
  interval_stream << "Function aves = " <<Fn_ave << '\n';
  interval_stream << "Function st devs = " <<Fn_stdevs << '\n';
  interval_stream << "2 sigma Credibility Intervals\n";
  for(size_t i=0; i<numFunctions; ++i){
    Cred_interval_minima[i] = Fn_ave[i] - 2*Fn_stdevs[i];
    Cred_interval_maxima[i] = Fn_ave[i] + 2*Fn_stdevs[i];
    interval_stream << std::setw(width) << resp[i] << " ";
    interval_stream << Cred_interval_minima[i] << ", " << Cred_interval_maxima[i] 
               << '\n';
  }    
  interval_stream << "\n";
  // Calculate +/- 2sigma prediction intervals
  RealVector Pred_ave(numFunctions), Pred_stdevs(numFunctions),
	     Pred_interval_minima(numFunctions), 
	     Pred_interval_maxima(numFunctions);
  compute_col_means(predVals_transpose, Pred_ave);
  compute_col_stdevs(predVals_transpose, Pred_ave, Pred_stdevs);
  interval_stream << "2 sigma Prediction Intervals\n";
  for(size_t i=0; i<numFunctions; ++i){
    Pred_interval_minima[i] = Pred_ave[i] - 2*Pred_stdevs[i];
    Pred_interval_maxima[i] = Pred_ave[i] + 2*Pred_stdevs[i];
    interval_stream << std::setw(width) << resp[i] << " ";
    interval_stream << Pred_interval_minima[i] << ", "<< Pred_interval_maxima[i] 
	       << '\n';
  }
  interval_stream << "\n";
  // Calculate intervals with sorting - print to screen and interval file
  size_t num_levels = 0;
  for(int i = 0; i < numFunctions; ++i){
    num_levels += requestedProbLevels[i].length();
  }
  if (num_levels > 0){
    print_intervals_file(interval_stream, filtered_fn_vals_transpose, 
      			   predVals_transpose, num_filtered, num_concatenated);
  }
}

void NonDBayesCalibration::compute_prediction_vals
(RealMatrix& filtered_fn_vals, RealMatrix& predVals, 
int num_filtered, size_t num_exp, size_t num_concatenated)
{
  // Read std_dev and correl matrices if specified for experiments
  RealVectorArray std_deviations;
  RealSymMatrixArray correl_matrices;
  if (calibrationData && expData.variance_active()){
    expData.cov_std_deviation(std_deviations);
    expData.cov_as_correlation(correl_matrices);
  }
  
  // Augment function values with experimental uncertainty for prediction ints
  // Generate normal errors using LHS
  /*int num_res = residualModel.current_response().num_functions();
    RealVector means_vec(num_res), lower_bnds(num_res), upper_bnds(num_res);
    */
  RealVector means_vec(numFunctions), lower_bnds(numFunctions), 
	     upper_bnds(numFunctions);
  means_vec.putScalar(0.0);
  lower_bnds.putScalar(-DBL_MAX);
  upper_bnds.putScalar(DBL_MAX);
  RealMatrix lhs_normal_samples;
  unsigned short sample_type = SUBMETHOD_LHS;
  short sample_ranks_mode = 0; //IGNORE RANKS
  Pecos::LHSDriver lhsDriver; // the C++ wrapper for the F90 LHS library
  int n = 0;
  lhsDriver.seed(randomSeed);
  lhsDriver.initialize("lhs", sample_ranks_mode, true);
  for(size_t i = 0; i < num_exp; ++i){
    //int lhs_seed = (randomSeed > 0) ? randomSeed : generate_system_seed();
    lhsDriver.generate_normal_samples(means_vec, std_deviations[i], lower_bnds,
              upper_bnds, num_filtered, correl_matrices[i],lhs_normal_samples);
    for(int j = 0; j < num_filtered; ++j){
      const RealVector& FnVal_colj = Teuchos::getCol(Teuchos::View, 
     			        filtered_fn_vals, j);
      const RealVector& lhs_colj = Teuchos::getCol(Teuchos::View, 
   				lhs_normal_samples, j);
      RealVector col_vec(numFunctions);
      for(size_t k = 0; k < numFunctions;++k){
        col_vec[k] = FnVal_colj[k] + lhs_colj[k];	
      }	
      //const RealVector& col_vec = Teuchos::getCol(Teuchos::View, 
      //filtered_fn_vals, j) + 
      //Teuchos::getCol(Teuchos::View, lhs_normal_samples, j);
      Teuchos::setCol(col_vec, n, predVals);
      n++; 
    }
  }
}

void NonDBayesCalibration::
compute_col_means(RealMatrix& matrix, RealVector& avg_vals)
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  avg_vals.resize(num_cols);
  
  RealVector ones_vec(num_rows);
  ones_vec.putScalar(1.0);
 
  for(int i=0; i<num_cols; ++i){
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    avg_vals(i) = col_vec.dot(ones_vec)/((Real) num_rows);
  }
}

void NonDBayesCalibration::
compute_col_stdevs(RealMatrix& matrix, RealVector& avg_vals, RealVector& std_devs)
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  std_devs.resize(num_cols);
  RealVector res_vec(num_rows);

  for(int i=0; i<num_cols; ++i){
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    for(int j = 0; j<num_rows; ++j){
      res_vec(j) = col_vec(j) - avg_vals(i);
    }
    std_devs(i) = std::sqrt(res_vec.dot(res_vec)/((Real) num_rows-1));
  }
}


// BMA TODO: the mcmc tabular and filtered tabular should be same
// format, probably same outputter...

void NonDBayesCalibration::export_chain()
{
  String mcmc_filename = 
    exportMCMCFilename.empty() ? "dakota_mcmc_tabular.dat" : exportMCMCFilename;
  std::ofstream export_mcmc_stream;
  TabularIO::open_file(export_mcmc_stream, mcmc_filename,
		       "NonDBayesCalibration chain export");

  // the residual model includes labels for the hyper-parameters, if present
  TabularIO::
    write_header_tabular(export_mcmc_stream, residualModel.current_variables(), 
			 StringArray(), "mcmc_id", exportMCMCFormat);
  //  size_t wpp4 = write_precision+4;
  export_mcmc_stream << std::setprecision(write_precision) 
		     << std::resetiosflags(std::ios::floatfield);

  // use a Variables object for proper tabular formatting
  Variables output_vars = residualModel.current_variables().copy();
  for (int j=0; j<acceptanceChain.numCols(); ++j) {
    String empty_id;
    TabularIO::write_leading_columns(export_mcmc_stream, j+1,
				     empty_id,//mcmcModel.interface_id(),
				     exportMCMCFormat);
    RealVector accept_pt = Teuchos::getCol(Teuchos::View, acceptanceChain, j);
    output_vars.continuous_variables(accept_pt);
    output_vars.write_tabular(export_mcmc_stream);
    export_mcmc_stream << '\n';
  }

  TabularIO::close_file(export_mcmc_stream, mcmc_filename,
			"NonDQUESOBayesCalibration chain export");
}


void NonDBayesCalibration::print_filtered_tabular(RealMatrix& filtered_chain, 
RealMatrix& filtered_fn_vals, RealMatrix& predVals, int num_filtered,
size_t num_exp)
{
  std::ofstream filtered_mcmc_stream;
  // Print tabular file with filtered chain, function values, and pred values
  String empty_id, filteredmcmc_filename = "dakota_mcmc_filtered_tabular.dat";
  TabularIO::open_file(filtered_mcmc_stream, filteredmcmc_filename,
 		       "NonDBayesCalibration filtered chain export");
  // When outputting only chain responses
  const StringArray& resp_array = mcmcModel.current_response().function_labels();
  // When outputting experimental responses
  /*StringArray resp_array;
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  for (size_t i=0; i<num_exp+1; ++i){
    for (size_t k=0; k<numFunctions; ++k){
      resp_array.push_back(resp[k]);
    }
  }*/
  Variables output_vars = residualModel.current_variables().copy();
  TabularIO::write_header_tabular(filtered_mcmc_stream, 
		            	  residualModel.current_variables(), resp_array, 
				  "mcmc_id", exportMCMCFormat);
  size_t sample_cntr = 0;
  size_t wpp4 = write_precision+4;
  for (int i=0; i<filtered_chain.numCols(); ++i) {
    String empty_id;
    TabularIO::write_leading_columns(filtered_mcmc_stream, i+1,
				     empty_id,//mcmcModel.interface_id(),
				     exportMCMCFormat);
    RealVector accept_pt = Teuchos::getCol(Teuchos::View, filtered_chain, i);
    output_vars.continuous_variables(accept_pt);
    output_vars.write_tabular(filtered_mcmc_stream);
    // Write function values to filtered_tabular
    RealVector col_vec = Teuchos::getCol(Teuchos::View, 
  			    	filtered_fn_vals, i);
    for (size_t j = 0; j<numFunctions; ++j){
      filtered_mcmc_stream << std::setw(wpp4) << col_vec[j] << ' ';
    }      
    // Write predicted values to filtered_tabular
    // When outputting experimental responses 
    /*for (size_t j =0; j<num_exp; ++j){
      for (size_t k = 0; k<numFunctions; ++k){
	int col_index = j*num_filtered+i;
        const RealVector& col_vec = Teuchos::getCol(Teuchos::View, 
      				    predVals, col_index);
  	filtered_mcmc_stream << std::setw(wpp4) << col_vec[k] << ' ';
      }
    }*/
    filtered_mcmc_stream << '\n';
  }
}

void NonDBayesCalibration::print_intervals_file
(std::ostream& s, RealMatrix& filteredFnVals_transpose, 
 RealMatrix& predVals_transpose, int num_filtered, size_t num_concatenated)
{
  
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  size_t width = write_precision+7;
  double alpha;
  int lower_index;
  int upper_index;
  // Credibility Intervals
  for(int i = 0; i < numFunctions; ++i){
    // Sort function values 
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View,
				filteredFnVals_transpose, i);
    std::sort(col_vec.values(), col_vec.values() + num_filtered);
    // Write intervals
    size_t num_prob_levels = requestedProbLevels[i].length();
    if (num_prob_levels > 0){
      s << "Credibility Intervals for ";
      s << resp[i] << '\n';
      s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
      s << std::setw(width) << ' ' << " ----------------- -----------------\n";
      for (size_t j = 0; j < num_prob_levels; ++j){
        alpha = requestedProbLevels[i][j];
        lower_index = floor(alpha/2*(num_filtered));
        upper_index = num_filtered - lower_index;
        s << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[lower_index] << ' ' << std::setw(width) 
	  << alpha << '\n'
	  << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[upper_index] << ' '<< std::setw(width) 
	  << 1-alpha << '\n'
	  << std::setw(width) << ' ' <<  "        -----             -----\n";
      }
    }
  }
  // Prediction Intervals
  for(int i = 0; i < numFunctions; ++i){
    // Sort function values 
    const RealVector& col_vec1 = Teuchos::getCol(Teuchos::View, 
				 predVals_transpose, i);
    std::sort(col_vec1.values(), col_vec1.values() + num_concatenated);
    // Write intervals
    size_t num_prob_levels = requestedProbLevels[i].length();
    if (num_prob_levels > 0){
      s << "Prediction Intervals for ";
      s << resp[i] << '\n';
      s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
      s << std::setw(width) << ' ' << " ----------------- -----------------\n";
      for (size_t j = 0; j < num_prob_levels; ++j){
        alpha = requestedProbLevels[i][j];
        //s << "alpha = " << alpha << '\n';
        lower_index = floor(alpha/2*(num_concatenated));
        upper_index = num_concatenated - lower_index;
        s << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec1[lower_index] << ' ' << std::setw(width) 
	  << alpha << '\n'
	  << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec1[upper_index] << ' '<< std::setw(width) 
	  << 1-alpha << '\n'
	  << std::setw(width) << ' ' <<  "        -----             -----\n";
      }
    }
  }
}

void NonDBayesCalibration::print_intervals_screen
(std::ostream& s, RealMatrix& filteredFnVals_transpose, 
 RealMatrix& predVals_transpose, int num_filtered)
{
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  size_t width = write_precision+7;
  double alpha;
  int lower_index;
  int upper_index;
  s << "\n";
  // Credibility Intervals
  for(int i = 0; i < numFunctions; ++i){
    // Sort function values 
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View,
				filteredFnVals_transpose, i);
    std::sort(col_vec.values(), col_vec.values() + num_filtered);
    // Write intervals
    size_t num_prob_levels = requestedProbLevels[i].length();
    if (num_prob_levels > 0){
      s << "Credibility Intervals for ";
      s << resp[i] << '\n';
      s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
      s << std::setw(width) << ' ' << " ----------------- -----------------\n";
      for (size_t j = 0; j < num_prob_levels; ++j){
        alpha = requestedProbLevels[i][j];
        lower_index = floor(alpha/2*(num_filtered));
        upper_index = num_filtered - lower_index;
        s << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[lower_index] << ' ' << std::setw(width) 
	  << alpha << '\n'
	  << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[upper_index] << ' '<< std::setw(width) 
	  << 1-alpha << '\n';
	  //<< std::setw(width) << ' ' <<  "        -----             -----\n";
      }
    }
  }
  // Prediction Intervals
  size_t num_exp = expData.num_experiments();
  size_t num_concatenated = num_exp*num_filtered;
  for(int i = 0; i < numFunctions; ++i){
    // Sort function values 
    const RealVector& col_vec1 = Teuchos::getCol(Teuchos::View, 
				 predVals_transpose, i);
    std::sort(col_vec1.values(), col_vec1.values() + num_concatenated);
    // Write intervals
    size_t num_prob_levels = requestedProbLevels[i].length();
    if (num_prob_levels > 0){
      s << "Prediction Intervals for ";
      s << resp[i] << '\n';
      s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
      s << std::setw(width) << ' ' << " ----------------- -----------------\n";
      for (size_t j = 0; j < num_prob_levels; ++j){
        alpha = requestedProbLevels[i][j];
        lower_index = floor(alpha/2*(num_concatenated));
        upper_index = num_concatenated - lower_index;
        s << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec1[lower_index] << ' ' << std::setw(width) 
	  << alpha << '\n'
	  << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec1[upper_index] << ' '<< std::setw(width) 
	  << 1-alpha << '\n';
	  //<< std::setw(width) << ' ' <<  "        -----             -----\n";
      }
    }
  }
}

void NonDBayesCalibration::print_results(std::ostream& s)
{
  // Print chain moments
  StringArray combined_labels;
        copy_data(residualModel.continuous_variable_labels(), combined_labels);
  NonDSampling::print_moments(s, chainStats, RealMatrix(), 
      "posterior variable", combined_labels, false); 
  // Print response moments
  StringArray resp_labels = mcmcModel.current_response().function_labels();
  NonDSampling::print_moments(s, fnStats, RealMatrix(), 
      "response function", resp_labels, false); 
  
  // Print credibility and prediction intervals to screen
  int num_filtered = filteredFnVals.numCols();
  RealMatrix filteredFnVals_transpose(filteredFnVals, Teuchos::TRANS);
  RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
  print_intervals_screen(s, filteredFnVals_transpose, 
    			 predVals_transpose, num_filtered);
}

} // namespace Dakota
