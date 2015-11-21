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
    probDescDB.get_string("method.nond.proposal_covariance_input_type"))
{
  // assign default proposalCovarType
  if (proposalCovarType.empty()) {
    if (emulatorType) proposalCovarType = "derivatives"; // misfit Hessian
    else              proposalCovarType = "prior";       // prior covariance
  }

  // manage sample partitions and defaults
  int samples_spec = probDescDB.get_int("method.samples");
  if (proposalCovarType == "derivatives") {
    int pc_update_spec
      = probDescDB.get_int("method.nond.proposal_covariance_updates");
    if (pc_update_spec < 1) { // default partition: update every 100 samples
      numSamples  = 100;
      chainCycles = (int)floor((Real)samples_spec / (Real)numSamples + .5);
    }
    else { // partition as specified
      numSamples  = (int)floor((Real)samples_spec / (Real)pc_update_spec + .5);
      chainCycles = pc_update_spec;
    }
  }
  else
    { numSamples = samples_spec; chainCycles = 1; }

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
    int samples = probDescDB.get_int("method.nond.emulator_samples");
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_build_points_file");
    if (!import_pts_file.empty())
      { samples = 0; sample_reuse = "all"; }
     
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator; Model lhs_model;
    if (standardizedSpace) transform_model(iteratedModel, lhs_model);//dist bnds
    else                   lhs_model = iteratedModel; // shared rep
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
  // Construct sampler for posterior stats (only in NonDQUESOBayes for now)
  // -------------------------------------
  //RealMatrix acceptance_chain; // empty (allSamples not currently needed)
  //chainStatsSampler.assign_rep(new NonDSampling(mcmcModel, acceptance_chain),
  //			         false);

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


//void NonDBayesCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
