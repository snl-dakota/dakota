/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
#include "ProbabilityTransformModel.hpp"
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
#include "WeightingModel.hpp"
#include "NonDMultilevelPolynomialChaos.hpp"
#include "NonDMultilevelStochCollocation.hpp"
#include "NonDLHSSampling.hpp"
#include "NPSOLOptimizer.hpp"
#include "SNLLOptimizer.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "LHSDriver.hpp"
// Using Boost MT since need it anyway for dist
#include "dakota_mersenne_twister.hpp"
#include "boost/random.hpp"
// Using Boost dist for cross-platform stability
#include "boost/random/normal_distribution.hpp"
#include "boost/random/variate_generator.hpp"
#include "boost/generator_iterator.hpp"
#include "boost/math/special_functions/digamma.hpp"
// BMA: May need to better manage DLL export / import from ANN in the future
// #ifdef DLL_EXPORTS
// #undef DLL_EXPORTS
// #endif
#include "ANN/ANN.h"
//#include "ANN/ANNperf.h"
//#include "ANN/ANNx.h"
#include "dakota_data_util.hpp"
//#include "dakota_tabular_io.hpp"
#include "DiscrepancyCorrection.hpp"
#include "bayes_calibration_utils.hpp"
#include "dakota_stat_util.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

enum miAlg : unsigned short {MI_ALG_KSG1 = 0, MI_ALG_KSG2 = 1};

// initialization of statics
NonDBayesCalibration* NonDBayesCalibration::nonDBayesInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::
NonDBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDCalibration(problem_db, model),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  mcmcModelHasSurrogate(false),
  mapOptAlgOverride(probDescDB.get_ushort("method.nond.opt_subproblem_solver")),
  chainSamples(probDescDB.get_int("method.nond.chain_samples")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  mcmcDerivOrder(1), batchSize(1), 
  adaptExpDesign(probDescDB.get_bool("method.nond.adapt_exp_design")),
  initHifiSamples (probDescDB.get_int("method.samples")),
  scalarDataFilename(probDescDB.get_string("responses.scalar_data_filename")),
  importCandPtsFile(
    probDescDB.get_string("method.import_candidate_points_file")),
  importCandFormat(
    probDescDB.get_ushort("method.import_candidate_format")),
  numCandidates(probDescDB.get_sizet("method.num_candidates")),
  maxHifiEvals(probDescDB.get_int("method.max_hifi_evaluations")),
  batchEvals(probDescDB.get_int("method.batch_size")),
  mutualInfoAlg(probDescDB.get_bool("method.nond.mutual_info_ksg2") ?
		MI_ALG_KSG2 : MI_ALG_KSG1),
  readFieldCoords(probDescDB.get_bool("responses.read_field_coordinates")),
  calModelDiscrepancy(probDescDB.get_bool("method.nond.model_discrepancy")),
  discrepancyType(probDescDB.get_string("method.nond.discrepancy_type")),
  numPredConfigs(probDescDB.get_sizet("method.num_prediction_configs")),
  predictionConfigList(probDescDB.get_rv("method.nond.prediction_configs")),
  importPredConfigs(probDescDB.get_string("method.import_prediction_configs")),
  importPredConfigFormat(
    probDescDB.get_ushort("method.import_prediction_configs_format")),
  exportCorrModelFile(
    probDescDB.get_string("method.nond.export_corrected_model_file")),
  exportCorrModelFormat(
    probDescDB.get_ushort("method.nond.export_corrected_model_format")),
  exportDiscrepFile(
    probDescDB.get_string("method.nond.export_discrepancy_file")),
  exportDiscrepFormat(
    probDescDB.get_ushort("method.nond.export_discrep_format")),
  exportCorrVarFile(
    probDescDB.get_string("method.nond.export_corrected_variance_file")),
  exportCorrVarFormat(
    probDescDB.get_ushort("method.nond.export_corrected_variance_format")),
  discrepPolyOrder(
    probDescDB.get_short("method.nond.model_discrepancy.polynomial_order")),
  // BMA: This is probably wrong as config vars need not be continuous!
  configLowerBnds(probDescDB.get_rv("variables.continuous_state.lower_bounds")),
  configUpperBnds(probDescDB.get_rv("variables.continuous_state.upper_bounds")),
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
  posteriorStatsMutual(
    probDescDB.get_bool("method.posterior_stats.mutual_info")),
  posteriorStatsKDE(probDescDB.get_bool("method.posterior_stats.kde")),
  chainDiagnostics(probDescDB.get_bool("method.chain_diagnostics")),
  chainDiagnosticsCI(probDescDB.get_bool("method.chain_diagnostics.confidence_intervals")),
  calModelEvidence(probDescDB.get_bool("method.model_evidence")),
  calModelEvidMC(probDescDB.get_bool("method.mc_approx")),
  calModelEvidLaplace(probDescDB.get_bool("method.laplace_approx")),
  evidenceSamples(probDescDB.get_int("method.evidence_samples")),
  subSamplingPeriod(probDescDB.get_int("method.sub_sampling_period")),
  exportMCMCFilename(
    probDescDB.get_string("method.nond.export_mcmc_points_file")),
  exportMCMCFormat(probDescDB.get_ushort("method.nond.export_samples_format")),
  scaleFlag(probDescDB.get_bool("method.scaling")),
  weightFlag(!iteratedModel.primary_response_fn_weights().empty())
{
  if (randomSeed)
    Cout << "NonDBayes Seed (user-specified) = "   << randomSeed << std::endl;
  else {
    randomSeed = generate_system_seed();
    Cout << "NonDBayes Seed (system-generated) = " << randomSeed << std::endl;
  }

  // NOTE: Burn-in defaults to 0 and sub-sampling to 1. We want to
  // allow chain_samples == 0 to perform map pre-solve only, so
  // account for that case here.
  if (burnInSamples > 0 && burnInSamples >= chainSamples) {
    Cerr << "\nError: burn_in_samples must be less than chain_samples.\n";
    abort_handler(PARSE_ERROR);
  }
  int num_filtered = 1 + (chainSamples - burnInSamples - 1)/subSamplingPeriod;
  Cout << "\nA chain of length " << chainSamples << " has been specified. "
       << burnInSamples << " burn in samples will be \ndiscarded and every "
       << subSamplingPeriod << "-th sample will be kept in the final chain. "
       << "The \nfinal chain will have length " << num_filtered << ".\n";

  if (adaptExpDesign) {
    // TODO: instead of pulling these models out, change modes on the
    // iteratedModel
    if (iteratedModel.model_type() != "surrogate") {
      Cerr << "\nError: Adaptive Bayesian experiment design requires " 
	   << "hierarchical surrogate\n       model.\n";
      abort_handler(PARSE_ERROR);
    }
    hifiModel = iteratedModel.truth_model();

    int num_exp = expData.num_experiments();
    int num_lhs_samples = std::max(initHifiSamples - num_exp, 0);
    // construct a hi-fi LHS sampler only if needed
    if (num_lhs_samples > 0) {
      unsigned short sample_type = SUBMETHOD_LHS;
      bool vary_pattern = true;
      String rng("mt19937");
      auto lhs_sampler_rep = std::make_shared<NonDLHSSampling>
	(hifiModel, sample_type, num_lhs_samples, randomSeed, rng, vary_pattern,
	 ACTIVE_UNIFORM);
      hifiSampler.assign_rep(lhs_sampler_rep);
    }
  }

  // assign default maxIterations (DataMethod default is SZ_MAX)
  if (adaptPosteriorRefine) {
    // BMA --> MSE: Why 5? Fix magic constant
    batchSize = 5;
    if (maxIterations == SZ_MAX) // default
      maxIterations = 25;
  }

  switch (emulatorType) {
  case PCE_EMULATOR:  case MF_PCE_EMULATOR:  case ML_PCE_EMULATOR:
  case  SC_EMULATOR:  case  MF_SC_EMULATOR:
    standardizedSpace = true; break; // nataf defined w/i ProbTransformModel
  default:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
    break;
  }

  // Errors if there are correlations and the user hasn't specified standardized_space,
  // since this is currently unsupported.
  // Note that gamma distribution should be supported but currently results in a seg fault.
  if ( !standardizedSpace && iteratedModel.multivariate_distribution().correlation() ){
    Cerr << "Error: correlation is only supported if user specifies standardized_space.\n"
      << "    Only the following types of correlated random variables are supported:\n"
      << "    unbounded normal, untruncated lognormal, uniform, exponential, gumbel, \n" 
      << "    frechet, and weibull."
	    << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Construct emulator objects for raw QoI, prior to data residual recast
  construct_mcmc_model();
  // define variable augmentation within residualModel
  init_hyper_parameters();

  // expand initial point by numHyperparams for use in negLogPostModel
  const Variables& orig_vars = iteratedModel.current_variables();
  size_t i, orig_cv_start = orig_vars.cv_start(), num_orig_cv = orig_vars.cv(),
    num_augment_cv = num_orig_cv + numHyperparams;
  mapSoln.sizeUninitialized(num_augment_cv);
  // allow mcmcModel to be in either distinct or all view
  copy_data_partial(mcmcModel.all_continuous_variables(), (int)orig_cv_start,
		    (int)num_orig_cv, mapSoln, 0);
  for (i=0; i<numHyperparams; ++i)
    mapSoln[num_orig_cv + i] = invGammaDists[i].mode();

  // Now the underlying simulation model mcmcModel is setup; wrap it
  // in a data transformation, making sure to allocate gradient/Hessian space
  const ShortShortPair& orig_view = orig_vars.view();
  if (calibrationData) {
    residualModel.assign_rep(std::make_shared<DataTransformModel>
			     (mcmcModel, expData, orig_view, numHyperparams,
			      obsErrorMultiplierMode, mcmcDerivOrder));
    // update bounds for hyper-parameters
    Real dbl_inf = std::numeric_limits<Real>::infinity();
    for (i=0; i<numHyperparams; ++i) { // ***
      residualModel.continuous_lower_bound(0.0,     numContinuousVars + i);
      residualModel.continuous_upper_bound(dbl_inf, numContinuousVars + i);
    }
  }
  else if (orig_view == mcmcModel.current_variables().view())
    residualModel = mcmcModel;  // shallow copy
  else // convert back from surrogate view to iteratedModel view for use in MCMC
    residualModel.assign_rep(
      std::make_shared<RecastModel>(mcmcModel, orig_view));

  // Order is important: data transform, then scale, then weights
  if (scaleFlag)   scale_model();
  if (weightFlag)  weight_model();

  init_map_optimizer();
  construct_map_model();

  Cout << "\n  iteratedModel num cv = " << iteratedModel.cv() << " mvd active = " << iteratedModel.multivariate_distribution().active_variables().count()
       << "\n  mcmcModel     num cv = " << mcmcModel.cv() << " mvd active = " << mcmcModel.multivariate_distribution().active_variables().count()
       << "\n  residualModel num cv = " << residualModel.cv() << " mvd active = " << residualModel.multivariate_distribution().active_variables().count() << std::endl;
  if (mapOptAlgOverride != SUBMETHOD_NONE)
    Cout << "\n  negLogPostModel num cv = "<< negLogPostModel.cv()<<std::endl;

  int mcmc_concurrency = 1; // prior to concurrent chains
  maxEvalConcurrency *= mcmc_concurrency;
}


void NonDBayesCalibration::construct_mcmc_model()
{
  // for adaptive experiment design, the surrogate model is the low-fi
  // model which should be calibrated
  // TODO: could avoid this lightweight copy entirely, but less clean
  Model inbound_model = 
    adaptExpDesign ? iteratedModel.surrogate_model() : iteratedModel;

  switch (emulatorType) {

  case PCE_EMULATOR: case ML_PCE_EMULATOR: case MF_PCE_EMULATOR:
  case  SC_EMULATOR: case  MF_SC_EMULATOR: {
    mcmcModelHasSurrogate = true;
    short u_space_type = probDescDB.get_short("method.nond.expansion_type");
    const RealVector& dim_pref
      = probDescDB.get_rv("method.nond.dimension_preference");
    short refine_type
        = probDescDB.get_short("method.nond.expansion_refinement_type"),
      refine_cntl
        = probDescDB.get_short("method.nond.expansion_refinement_control"),
      cov_cntl
        = probDescDB.get_short("method.nond.covariance_control"),
      rule_nest = probDescDB.get_short("method.nond.nesting_override"),
      rule_growth = probDescDB.get_short("method.nond.growth_override");
    bool pw_basis = probDescDB.get_bool("method.nond.piecewise_basis"),
       use_derivs = probDescDB.get_bool("method.derivative_usage");
    std::shared_ptr<NonDExpansion> se_rep;

    if (emulatorType == SC_EMULATOR) { // SC sparse grid interpolation
      unsigned short ssg_level
	= probDescDB.get_ushort("method.nond.sparse_grid_level");
      unsigned short tpq_order
	= probDescDB.get_ushort("method.nond.quadrature_order");
      if (ssg_level != USHRT_MAX) {
	short exp_coeff_approach = Pecos::COMBINED_SPARSE_GRID;
	if (probDescDB.get_short("method.nond.expansion_basis_type") ==
	    Pecos::HIERARCHICAL_INTERPOLANT)
	  exp_coeff_approach = Pecos::HIERARCHICAL_SPARSE_GRID;
	else if (refine_cntl)
	  exp_coeff_approach = Pecos::INCREMENTAL_SPARSE_GRID;
	se_rep = std::make_shared<NonDStochCollocation>(inbound_model,
	  exp_coeff_approach, ssg_level, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, rule_nest, rule_growth, pw_basis, use_derivs);
      }
      else if (tpq_order != USHRT_MAX)
	se_rep = std::make_shared<NonDStochCollocation>(inbound_model,
	  Pecos::QUADRATURE, tpq_order, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, rule_nest, rule_growth, pw_basis, use_derivs);
      mcmcDerivOrder = 3; // Hessian computations not yet implemented for SC
    }

    else if (emulatorType == PCE_EMULATOR) {
      const String& exp_import_file
	= probDescDB.get_string("method.nond.import_expansion_file");
      const String& exp_export_file
        = probDescDB.get_string("method.nond.export_expansion_file");
      unsigned short ssg_level
	= probDescDB.get_ushort("method.nond.sparse_grid_level");
      unsigned short tpq_order
	= probDescDB.get_ushort("method.nond.quadrature_order");
      unsigned short cub_int
	= probDescDB.get_ushort("method.nond.cubature_integrand");
      if (!exp_import_file.empty()) {
	// While upstream update allows NonD ctor chain to use updated number
	// of active CV, we should avoid modifying the original calibration
	// configuration provided by the incoming iteratedModel.  Rather, we
	// must adjust downstream within the PCE ctor.
	//if (expData.num_config_vars())
	//  inbound_model.active_view(MIXED_ALL); // allow recursion

	// Imported surrogate will include state config vars for now.
	// TO DO: expand this override to non-imported cases.
	ShortShortPair approx_view(MIXED_ALL, EMPTY_VIEW);
	se_rep = std::make_shared<NonDPolynomialChaos>(inbound_model,
	  exp_import_file, u_space_type, approx_view);//no export since imported
      }
      else if (ssg_level != USHRT_MAX) { // PCE sparse grid
	short exp_coeff_approach = (refine_cntl) ?
	  Pecos::INCREMENTAL_SPARSE_GRID : Pecos::COMBINED_SPARSE_GRID;
	se_rep = std::make_shared<NonDPolynomialChaos>(inbound_model,
	  exp_coeff_approach, ssg_level, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, rule_nest, rule_growth, pw_basis, use_derivs,
	  exp_export_file);
      }
      else if (tpq_order != USHRT_MAX)
	se_rep = std::make_shared<NonDPolynomialChaos>(inbound_model,
	  Pecos::QUADRATURE, tpq_order, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, rule_nest, rule_growth, pw_basis, use_derivs,
	  exp_export_file);
      else if (cub_int != USHRT_MAX)
	se_rep = std::make_shared<NonDPolynomialChaos>(inbound_model,
	  Pecos::CUBATURE, cub_int, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, rule_nest, rule_growth, pw_basis, use_derivs,
	  exp_export_file);
      else { // regression PCE: LeastSq/CS, OLI
	se_rep = std::make_shared<NonDPolynomialChaos>(inbound_model,
	  probDescDB.get_short("method.nond.regression_type"), 
	  probDescDB.get_ushort("method.nond.expansion_order"), dim_pref,
	  probDescDB.get_sizet("method.nond.collocation_points"),
	  probDescDB.get_real("method.nond.collocation_ratio"), // single scalar
	  randomSeed, u_space_type, refine_type, refine_cntl, cov_cntl,
	  /* rule_nest, rule_growth, */ pw_basis, use_derivs,
	  probDescDB.get_bool("method.nond.cross_validation"),
	  probDescDB.get_string("method.import_build_points_file"),
	  probDescDB.get_ushort("method.import_build_format"),
	  probDescDB.get_bool("method.import_build_active_only"),
	  exp_export_file);
      }
      mcmcDerivOrder = 7; // Hessian computations implemented for PCE
    }

    else if (emulatorType == MF_SC_EMULATOR) {
      const UShortArray& ssg_level_seq
	= probDescDB.get_usa("method.nond.sparse_grid_level");
      const UShortArray& tpq_order_seq
	= probDescDB.get_usa("method.nond.quadrature_order");
      short ml_alloc_cntl
	= probDescDB.get_short("method.nond.multilevel_allocation_control"),
	ml_discrep
	= probDescDB.get_short("method.nond.multilevel_discrepancy_emulation");
      if (!ssg_level_seq.empty()) {
	short exp_coeff_approach = Pecos::COMBINED_SPARSE_GRID;
	if (probDescDB.get_short("method.nond.expansion_basis_type") ==
	    Pecos::HIERARCHICAL_INTERPOLANT)
	  exp_coeff_approach = Pecos::HIERARCHICAL_SPARSE_GRID;
	else if (refine_cntl)
	  exp_coeff_approach = Pecos::INCREMENTAL_SPARSE_GRID;
	se_rep = std::make_shared<NonDMultilevelStochCollocation>(inbound_model,
	  exp_coeff_approach, ssg_level_seq, dim_pref, u_space_type,
	  refine_type, refine_cntl, cov_cntl, ml_alloc_cntl, ml_discrep,
	  rule_nest, rule_growth, pw_basis, use_derivs);
      }
      else if (!tpq_order_seq.empty())
	se_rep = std::make_shared<NonDMultilevelStochCollocation>(inbound_model,
	  Pecos::QUADRATURE, tpq_order_seq, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, ml_alloc_cntl, ml_discrep, rule_nest,
	  rule_growth, pw_basis, use_derivs);
      mcmcDerivOrder = 3; // Hessian computations not yet implemented for SC
    }

    else if (emulatorType == MF_PCE_EMULATOR) {
      const UShortArray& ssg_level_seq
	= probDescDB.get_usa("method.nond.sparse_grid_level");
      const UShortArray& tpq_order_seq
	= probDescDB.get_usa("method.nond.quadrature_order");
      short ml_alloc_cntl
	= probDescDB.get_short("method.nond.multilevel_allocation_control"),
	ml_discrep
	= probDescDB.get_short("method.nond.multilevel_discrepancy_emulation");
      if (!ssg_level_seq.empty()) {
	short exp_coeff_approach = (refine_cntl) ?
	  Pecos::INCREMENTAL_SPARSE_GRID : Pecos::COMBINED_SPARSE_GRID;
	se_rep = std::make_shared<NonDMultilevelPolynomialChaos>(inbound_model,
	  exp_coeff_approach, ssg_level_seq, dim_pref, u_space_type,
	  refine_type, refine_cntl, cov_cntl, ml_alloc_cntl, ml_discrep,
	  rule_nest, rule_growth, pw_basis, use_derivs);
      }
      else if (!tpq_order_seq.empty())
	se_rep = std::make_shared<NonDMultilevelPolynomialChaos>(inbound_model,
	  Pecos::QUADRATURE, tpq_order_seq, dim_pref, u_space_type, refine_type,
	  refine_cntl, cov_cntl, ml_alloc_cntl, ml_discrep, rule_nest,
	  rule_growth, pw_basis, use_derivs);
      else { // regression PCE: LeastSq/CS, OLI
	SizetArray seed_seq(1, randomSeed); // reuse bayes_calib scalar spec
	se_rep = std::make_shared<NonDMultilevelPolynomialChaos>(
	  MULTIFIDELITY_POLYNOMIAL_CHAOS, inbound_model,
	  probDescDB.get_short("method.nond.regression_type"), 
	  probDescDB.get_usa("method.nond.expansion_order"), dim_pref,
	  probDescDB.get_sza("method.nond.collocation_points"), // sequence
	  probDescDB.get_real("method.nond.collocation_ratio"), // scalar
	  seed_seq, u_space_type, refine_type, refine_cntl, cov_cntl,
	  ml_alloc_cntl, ml_discrep, /* rule_nest, rule_growth, */ pw_basis,
	  use_derivs, probDescDB.get_bool("method.nond.cross_validation"),
	  probDescDB.get_string("method.import_build_points_file"),
	  probDescDB.get_ushort("method.import_build_format"),
	  probDescDB.get_bool("method.import_build_active_only"));
      }
      mcmcDerivOrder = 7; // Hessian computations implemented for PCE
    }

    else if (emulatorType == ML_PCE_EMULATOR) {
      SizetArray seed_seq(1, randomSeed); // reuse bayes_calib scalar spec
      se_rep = std::make_shared<NonDMultilevelPolynomialChaos>(
	MULTILEVEL_POLYNOMIAL_CHAOS, inbound_model,
	probDescDB.get_short("method.nond.regression_type"),
	probDescDB.get_usa("method.nond.expansion_order"), dim_pref,
	probDescDB.get_sza("method.nond.collocation_points"), // sequence
	probDescDB.get_real("method.nond.collocation_ratio"), // scalar
	seed_seq, u_space_type, refine_type, refine_cntl, cov_cntl,
	probDescDB.get_short("method.nond.multilevel_allocation_control"),
	probDescDB.get_short("method.nond.multilevel_discrepancy_emulation"),
	/* rule_nest, rule_growth, */ pw_basis, use_derivs,
	probDescDB.get_bool("method.nond.cross_validation"),
	probDescDB.get_string("method.import_build_points_file"),
	probDescDB.get_ushort("method.import_build_format"),
	probDescDB.get_bool("method.import_build_active_only"));
      mcmcDerivOrder = 7; // Hessian computations implemented for PCE
    }

    // for adaptive exp refinement, propagate controls from Bayes method spec:
    se_rep->maximum_iterations(maxIterations);
    se_rep->maximum_refinement_iterations(
      probDescDB.get_sizet("method.nond.max_refinement_iterations"));
    se_rep->convergence_tolerance(convergenceTol);

    stochExpIterator.assign_rep(se_rep);
    // no CDF or PDF level mappings
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
      empty_rv_array, respLevelTarget, respLevelTargetReduce, cdfFlag, false);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    mcmcModel = stochExpIterator.algorithm_space_model(); // shared rep
    break;
  }

  case GP_EMULATOR: case KRIGING_EMULATOR: {
    mcmcModelHasSurrogate = true;
    String sample_reuse; String approx_type;
    if (emulatorType == GP_EMULATOR)
      { approx_type = "global_gaussian"; mcmcDerivOrder = 3; } // grad support
    else
      { approx_type = "global_kriging";  mcmcDerivOrder = 7; } // grad,Hess
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
    if (probDescDB.get_bool("method.derivative_usage")) {
      // derivatives for emulator construction (not emulator evaluation)
      if (inbound_model.gradient_type() != "none") data_order |= 2;
      if (inbound_model.hessian_type()  != "none") data_order |= 4;
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
      lhs_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
	inbound_model, ASKEY_U, inbound_model.current_variables().view(),
	truncate_bnds)); //, 3.)
    else
      lhs_model = inbound_model; // shared rep
    // Unlike EGO-based approaches, use ACTIVE sampling mode to concentrate
    // samples in regions of higher prior density
    auto lhs_rep = std::make_shared<NonDLHSSampling>(lhs_model, sample_type,
      samples, randomSeed,
      probDescDB.get_string("method.random_number_generator"));
    lhs_iterator.assign_rep(lhs_rep);

    ActiveSet gp_set = lhs_model.current_response().active_set(); // copy
    gp_set.request_values(mcmcDerivOrder); // for misfit Hessian
    const ShortShortPair& gp_view = lhs_model.current_variables().view();
    mcmcModel.assign_rep(std::make_shared<DataFitSurrModel>(lhs_iterator,
      lhs_model, gp_set, gp_view, approx_type, approx_order, corr_type,
      corr_order, data_order, outputLevel, sample_reuse, import_pts_file,
      probDescDB.get_ushort("method.import_build_format"),
      probDescDB.get_bool("method.import_build_active_only")));
    break;
  }

  case NO_EMULATOR:
    mcmcModelHasSurrogate = (inbound_model.model_type() == "surrogate");
    // ASKEY_U is currently the best option for scaling the probability space
    // (but could be expanded when the intent is not orthogonal polynomials).
    // If an override is needed to decorrelate priors be transforming to
    // STD_NORMAL space, this is managed by ProbabilityTransformModel::
    // verify_correlation_support() on a variable-by-variable basis.
    if (standardizedSpace)
      mcmcModel.assign_rep(std::make_shared<ProbabilityTransformModel>(
	inbound_model, ASKEY_U, inbound_model.current_variables().view()));
    else
      mcmcModel = inbound_model; // shared rep

    if (mcmcModel.gradient_type() != "none") mcmcDerivOrder |= 2;
    if (mcmcModel.hessian_type()  != "none") mcmcDerivOrder |= 4;
    break;
  }
}


void NonDBayesCalibration::init_hyper_parameters()
{
  // Initialize sizing for hyperparameters (observation error), not
  // currently part of a RecastModel
  size_t num_resp_groups = 
    mcmcModel.current_response().shared_data().num_response_groups(); 
  if (obsErrorMultiplierMode == CALIBRATE_ONE)
    numHyperparams = 1;
  else if (obsErrorMultiplierMode == CALIBRATE_PER_EXPER)
    numHyperparams = expData.num_experiments();
  else if (obsErrorMultiplierMode == CALIBRATE_PER_RESP)
    numHyperparams = num_resp_groups;
  else if (obsErrorMultiplierMode == CALIBRATE_BOTH)
    numHyperparams = expData.num_experiments() * num_resp_groups;

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
    std::shared_ptr<Pecos::InvGammaRandomVariable> rv_rep =
      std::static_pointer_cast<Pecos::InvGammaRandomVariable>
	     (invGammaDists[i].random_variable_rep());
    rv_rep->update(alpha, beta);
  }
}


/** Construct optimizer for MAP pre-solve
    Emulator:     on by default; can be overridden with "pre_solve none"
    No emulator: off by default; can be activated  with "pre_solve {sqp,nip}"
                 relies on mapOptimizer ctor to enforce min derivative support
    Calculation of model evidence using Laplace approximation: 
		 this requires a MAP solve. 
*/
void NonDBayesCalibration::init_map_optimizer() 
{
  switch (mapOptAlgOverride) {
  case SUBMETHOD_NPSOL:
#ifndef HAVE_NPSOL
    Cerr << "\nWarning: this executable not configured with NPSOL SQP."
	 << "\n         MAP pre-solve not available." << std::endl;
    mapOptAlgOverride = SUBMETHOD_NONE; // model,optimizer not constructed
#endif
    break;
  case SUBMETHOD_OPTPP:
#ifndef HAVE_OPTPP
    Cerr << "\nWarning: this executable not configured with OPT++ NIP."
	 << "\n         MAP pre-solve not available." << std::endl;
    mapOptAlgOverride = SUBMETHOD_NONE; // model,optimizer not constructed
#endif
    break;
  case SUBMETHOD_DEFAULT: // use full Newton, if available
    if (emulatorType || calModelEvidLaplace) {
#ifdef HAVE_OPTPP
      mapOptAlgOverride = SUBMETHOD_OPTPP;
#elif HAVE_NPSOL
      mapOptAlgOverride = SUBMETHOD_NPSOL;
#else
      mapOptAlgOverride = SUBMETHOD_NONE;
#endif
    }
    break;
  //case SUBMETHOD_NONE:
  //  break;
  }

  // Errors / warnings
  if (mapOptAlgOverride == SUBMETHOD_NONE) {
    if (calModelEvidLaplace) {
      Cout << "Error: You must specify a pre-solve method for the Laplace "
	   << "approximation of model evidence." << std::endl; 
      abort_handler(METHOD_ERROR);
    }
    if (emulatorType)
      Cerr << "\nWarning: this executable not configured with NPSOL or OPT++."
	   << "\n         MAP pre-solve not available." << std::endl;
  }      
}


void NonDBayesCalibration::construct_map_model() 
{
  if (mapOptAlgOverride == SUBMETHOD_NONE) return;

  size_t num_total_calib_terms = residualModel.num_primary_fns();
  Sizet2DArray vars_map_indices, primary_resp_map_indices(1),
    secondary_resp_map_indices;
  primary_resp_map_indices[0].resize(num_total_calib_terms);
  for (size_t i=0; i<num_total_calib_terms; ++i)
    primary_resp_map_indices[0][i] = i;
  bool nonlinear_vars_map = false; BoolDequeArray nonlinear_resp_map(1);
  nonlinear_resp_map[0] = BoolDeque(num_total_calib_terms, true);
  SizetArray recast_vc_totals;  // empty: no change in size
  BitArray all_relax_di, all_relax_dr; // empty: no discrete relaxation

  // recast ActiveSet requests if full-Newton NIP with gauss-Newton misfit
  // (avoids error in unsupported Hessian requests in Model::manage_asv())
  short nlp_resp_order = 3; // quasi-Newton optimization
  void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
  if (mapOptAlgOverride == SUBMETHOD_OPTPP) {
    nlp_resp_order = 7; // size RecastModel response for full Newton Hessian
    if (mcmcDerivOrder == 3) // map asrv for Gauss-Newton approx
      set_recast = gnewton_set_recast;
  }

  // RecastModel for bound-constrained argmin(misfit - log prior)
  negLogPostModel.assign_rep(std::make_shared<RecastModel>(residualModel,
    vars_map_indices, recast_vc_totals, all_relax_di, all_relax_dr,
    nonlinear_vars_map, iteratedModel.current_variables().view(), nullptr,
    set_recast, primary_resp_map_indices, secondary_resp_map_indices, 0,
    nlp_resp_order, nonlinear_resp_map, neg_log_post_resp_mapping, nullptr));
}

void NonDBayesCalibration::construct_map_optimizer() 
{
  // Note: this fn may get invoked repeatedly but assign_rep() manages the
  // outgoing pointer

  switch (mapOptAlgOverride) {
#ifdef HAVE_NPSOL
  case SUBMETHOD_NPSOL: {
    // SQP with BFGS Hessians
    int npsol_deriv_level = 3;
    mapOptimizer.assign_rep(std::make_shared<NPSOLOptimizer>
			    (negLogPostModel, npsol_deriv_level, convergenceTol));
    break;
  }
#endif
#ifdef HAVE_OPTPP
  case SUBMETHOD_OPTPP:
    // full Newton (OPTPP::OptBCNewton)
    mapOptimizer.assign_rep(std::make_shared<SNLLOptimizer>
			    ("optpp_newton", negLogPostModel));
    break;
#endif
  }
}


NonDBayesCalibration::~NonDBayesCalibration()
{ }


void NonDBayesCalibration::pre_run()
{
  Analyzer::pre_run();

  // now that vars/labels/bounds/targets have flowed down at run-time from
  // any higher level recursions, propagate them up local Model recursions
  // so that they are correct when they propagate back down.
  // *** TO DO: count Model recursion layers on top of iteratedModel 
  if (!negLogPostModel.is_null())
    negLogPostModel.update_from_subordinate_model(); // depth = max
  else
    residualModel.update_from_subordinate_model();   // depth = max

  // needs to follow bounds updates so that correct OPT++ optimizer is selected
  // (OptBCNewtonLike or OptNewtonLike)
  construct_map_optimizer();
}

void NonDBayesCalibration::core_run()
{
  nonDBayesInstance = this;

  specify_prior();
  initialize_model();
  specify_likelihood();
  specify_posterior();  
  init_bayesian_solver();

  if (adaptExpDesign) // use meta-iteration in this class
    calibrate_to_hifi();
  else if (adaptPosteriorRefine)
    calibrate_with_adaptive_emulator();
  else                // delegate to base class calibration
    calibrate();

  // May need to override this in some child classes without chains
  compute_statistics();

  if (calModelDiscrepancy) // calibrate a model discrepancy function
    build_model_discrepancy();
    //print_discrepancy_results();
}

void NonDBayesCalibration::derived_init_communicators(ParLevLIter pl_iter)
{
  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR:    case SC_EMULATOR:
  case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: case MF_SC_EMULATOR:
    stochExpIterator.init_communicators(pl_iter);              break;
  //default:
  //  mcmcModel.init_communicators(pl_iter, maxEvalConcurrency); break;
  }
  residualModel.init_communicators(pl_iter, maxEvalConcurrency);

  if (!mapOptimizer.is_null())
    mapOptimizer.init_communicators(pl_iter);

  if (!hifiSampler.is_null())
    hifiSampler.init_communicators(pl_iter);
}


void NonDBayesCalibration::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR:    case SC_EMULATOR:
  case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: case MF_SC_EMULATOR:
    stochExpIterator.set_communicators(pl_iter);              break;
  //default:
  //  mcmcModel.set_communicators(pl_iter, maxEvalConcurrency); break;
  }
  residualModel.set_communicators(pl_iter, maxEvalConcurrency);

  if (!mapOptimizer.is_null())
    mapOptimizer.set_communicators(pl_iter);

  if (!hifiSampler.is_null())
    hifiSampler.set_communicators(pl_iter);
}


void NonDBayesCalibration::derived_free_communicators(ParLevLIter pl_iter)
{
  if (!hifiSampler.is_null())
    hifiSampler.free_communicators(pl_iter);

  if (!mapOptimizer.is_null())
    mapOptimizer.free_communicators(pl_iter);

  residualModel.free_communicators(pl_iter, maxEvalConcurrency);
  switch (emulatorType) {
  case PCE_EMULATOR:    case SC_EMULATOR:
  case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: case MF_SC_EMULATOR:
    stochExpIterator.free_communicators(pl_iter);              break;
  //default:
  //  mcmcModel.free_communicators(pl_iter, maxEvalConcurrency); break;
  }
}


void NonDBayesCalibration::initialize_model()
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
  case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: case MF_SC_EMULATOR: {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    stochExpIterator.run(pl_iter); break;
  }
  default: // GPs and NO_EMULATOR
    //resize_final_statistics_gradients(); // not required
    if (emulatorType)
      mcmcModel.build_approximation();
    break;
  }
  if (posteriorStatsMutual)
    Cout << "Mutual Information estimation not yet implemented\n";
}


void NonDBayesCalibration::map_pre_solve()
{
  /// Runs a pre-solve for the MAP point. If running calibrate_to_hifi()
  /// or calibrate_with_adaptive_emulator(), propagates the solution to
  /// the mapSoln variable. Returns the optimal solution as a vector.

  // Management of pre_solve spec options occurs in NonDBayesCalibration ctor,
  // manifesting here as a valid mapOptimizer instance.
  if (mapOptimizer.is_null()) return;
  
  // Pre-solve for MAP point using optimization prior to MCMC.

  Cout << "\nInitiating pre-solve for maximum a posteriori probability (MAP)."
       << std::endl;
  // set initial point pulled from mcmcModel at construct time or
  // warm start from previous map soln computed from previous emulator
  negLogPostModel.current_variables().continuous_variables(mapSoln);

  mapOptimizer.run();
  //negLogPostModel.print_evaluation_summary(Cout);
  //mapOptimizer.print_results(Cout); // needs xform if standardizedSpace
  Cout << "Maximum a posteriori probability (MAP) point from pre-solve"
       << "\n(will be used as initial point for MCMC chain):\n";

  // TNP ? Why are we introducting this local variable instead of copying to
  // mapSoln?
  const RealVector& map_c_vars
    = mapOptimizer.variables_results().continuous_variables();
  print_variables(Cout, map_c_vars);
  Cout << std::endl;

  // TNP ? Switched to propagating to mapSoln no matter what--is that violating
  // some underlying assumption?
  
  // if multiple pre-solves, propagate MAP as initial guess for next pre-solve
  //if (adaptPosteriorRefine || adaptExpDesign)
    copy_data(map_c_vars, mapSoln); // deep copy of view

}

void NonDBayesCalibration::calibrate_with_adaptive_emulator(){
  /// This method will perform a Bayesian calibration with an emulator, 
  /// but periodically the emulator is updated with more sample points from the 
  /// original model in the high-posterior-density region of parameter space.
  
  // TNP ? This seems like it maybe should be happening in ctor?
  if (!emulatorType) { // current spec prevents this
    Cerr << "Error: adaptive posterior refinement requires emulator model."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  compactMode = true; // update_model() uses all{Samples,Responses}
  Real adapt_metric = DBL_MAX; unsigned short int num_mcmc = 0;
  while (adapt_metric > convergenceTol && num_mcmc <= maxIterations) {

    // TO DO: treat this like cross-validation as there is likely a sweet
    // spot prior to degradation of conditioning (too much refinement data)

    // place update block here so that chain is always run for initial or
    // updated emulator; placing block at loop end could result in emulator
    // convergence w/o final chain.
    if (num_mcmc) {
	    // update the emulator surrogate data with new truth evals and
	    // reconstruct surrogate (e.g., via PCE sparse recovery)
	    update_model();
	    // assess posterior convergence via convergence of the emulator coeffs
	    adapt_metric = assess_emulator_convergence();
    }

    calibrate();

    // populate allSamples for surrogate updating
    if (emulatorType == PCE_EMULATOR)
      filter_chain_by_conditioning();
    else
      best_to_all();
    
    ++num_mcmc;

    // assess convergence of the posterior via sample-based K-L divergence:
    //adapt_metric = assess_posterior_convergence();
  } // adapt while
}

void NonDBayesCalibration::best_to_all()
{
  if (outputLevel >= NORMAL_OUTPUT) Cout << "Chain filtering results:\n";

  int num_best = bestSamples.size();
  if (allSamples.numCols() != num_best)
    allSamples.shapeUninitialized(numContinuousVars, num_best);

  std::/*multi*/map<Real, RealVector>::const_iterator
    bs_it = bestSamples.begin(), bs_end = bestSamples.end();
  for (int i=0; bs_it != bs_end; ++bs_it, ++i) {
    Teuchos::setCol(bs_it->second, i, allSamples);
    if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "Best point " << i+1 << ": Log posterior = " << bs_it->first
	   << " Sample:";
      // BMA TODO: vector writer?
      //      Cout << bs_it->second;
      write_col_vector_trans(Cout, (int)i, allSamples, false, false, true);
    }
  }
}


void NonDBayesCalibration::update_model()
{
  if (!emulatorType) {
    Cerr << "Error: NonDBayesCalibration::update_model() requires an "
	 << "emulator model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // perform truth evals (in parallel) for selected points
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Updating emulator: evaluating " << allSamples.numCols()
	 << " best points." << std::endl;
  // bypass surrogate but preserve transformations to standardized space
  short orig_resp_mode = mcmcModel.surrogate_response_mode(); // store mode
  mcmcModel.surrogate_response_mode(BYPASS_SURROGATE); // actual model evals
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
  case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: case MF_SC_EMULATOR:
    nondInstance = (NonD*)stochExpIterator.iterator_rep().get();
    evaluate_parameter_sets(mcmcModel, true, false); // log allResp, no best
    nondInstance = this; // restore
    break;
  case GP_EMULATOR: case KRIGING_EMULATOR:
    if (standardizedSpace)
      nondInstance
	= (NonD*)mcmcModel.subordinate_iterator().iterator_rep().get();
    evaluate_parameter_sets(mcmcModel, true, false); // log allResp, no best
    if (standardizedSpace)
      nondInstance = this; // restore
    break;
  }
  mcmcModel.surrogate_response_mode(orig_resp_mode); // restore mode

  // update mcmcModel with new data from iteratedModel
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Updating emulator: appending " << allResponses.size()
	 << " new data sets." << std::endl;
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
  case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: case MF_SC_EMULATOR: {
    // Adapt the expansion in sync with the dataset using a top-down design
    // (more explicit than embedded logic w/i mcmcModel.append_approximation).
    std::shared_ptr<NonDExpansion> se_iterator =
      std::static_pointer_cast<NonDExpansion>(stochExpIterator.iterator_rep());
    se_iterator->append_expansion(allSamples, allResponses);
    // TO DO: order increment places addtnl reqmts on emulator conv assessment
    break;
  }
  case GP_EMULATOR: case KRIGING_EMULATOR:
    mcmcModel.append_approximation(allSamples, allResponses, true); // rebuild
    break;
  }
}


Real NonDBayesCalibration::assess_emulator_convergence()
{
  // coeff reference point not yet available; force another iteration rather
  // than use norm of current coeffs (stopping on small norm is not meaningful)
  if (prevCoeffs.empty()) {
    switch (emulatorType) {
    case PCE_EMULATOR: case ML_PCE_EMULATOR: case MF_PCE_EMULATOR:
      prevCoeffs = mcmcModel.approximation_coefficients(true);  break;
    case SC_EMULATOR: case MF_SC_EMULATOR:
      prevCoeffs = mcmcModel.approximation_coefficients(false); break;
    case GP_EMULATOR: case KRIGING_EMULATOR:
      Cerr << "Warning: convergence norm not yet defined for GP emulators in "
	   << "NonDBayesCalibration::assess_emulator_convergence()."
	   << std::endl;
      break;
    }
    return DBL_MAX;
  }

  Real l2_norm_delta_coeffs = 0., delta_coeff_ij;
  switch (emulatorType) {
  case PCE_EMULATOR: case ML_PCE_EMULATOR: case MF_PCE_EMULATOR: {
    // normalized coeffs:
    const RealVectorArray& coeffs = mcmcModel.approximation_coefficients(true);
    size_t i, j, num_qoi = coeffs.size(),
      num_curr_coeffs, num_prev_coeffs, num_coeffs;

    // This approach assumes a well-ordered progression in multiIndex, which is
    // acceptable for regression PCE using consistently incremented (candidate)
    // expansion definitions.  Sparsity is not a concern as returned coeffs are
    // inflated to be dense w.r.t. SharedOrthogPolyApproxData::multiIndex.
    // Could implement as resize (inflat smaller w/ 0's) + vector difference +
    // Frobenious norm, but current approach should have lower overhead.
    for (i=0; i<num_qoi; ++i) {
      const RealVector&      coeffs_i =     coeffs[i];
      const RealVector& prev_coeffs_i = prevCoeffs[i];
      num_curr_coeffs = coeffs_i.length();
      num_prev_coeffs = prev_coeffs_i.length();
      num_coeffs = std::max(num_curr_coeffs, num_prev_coeffs);
      for (j=0; j<num_coeffs; ++j) {
	delta_coeff_ij = 0.;
	if (j<num_curr_coeffs) delta_coeff_ij += coeffs_i[j];
	if (j<num_prev_coeffs) delta_coeff_ij -= prev_coeffs_i[j];
	l2_norm_delta_coeffs += delta_coeff_ij * delta_coeff_ij;
      }
    }

    prevCoeffs = coeffs;
    break;
  }
  case SC_EMULATOR: case MF_SC_EMULATOR: {
    // Interpolation could use a similar concept with the expansion coeffs,
    // although adaptation would imply differences in the grid.
    const RealVectorArray& coeffs = mcmcModel.approximation_coefficients(false);

    Cerr << "Warning: convergence norm not yet defined for SC emulator in "
	 << "NonDBayesCalibration::assess_emulator_convergence()."
	 << std::endl;
    //abort_handler(METHOD_ERROR);
    return DBL_MAX;
    break;
  }
  case GP_EMULATOR: case KRIGING_EMULATOR:
    // Consider use of correlation lengths.
    // TO DO: define SurfpackApproximation::approximation_coefficients()...
    Cerr << "Warning: convergence norm not yet defined for GP emulators in "
	 << "NonDBayesCalibration::assess_emulator_convergence()."
	 << std::endl;
    //abort_handler(METHOD_ERROR);
    return DBL_MAX;
    break;
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    Real norm = std::sqrt(l2_norm_delta_coeffs);
    Cout << "Assessing emulator convergence: l2 norm = " << norm << std::endl;
    return norm;
  }
  else
    return std::sqrt(l2_norm_delta_coeffs);
} // assess_emulator_convergence


void NonDBayesCalibration::calibrate_to_hifi()
{
  /* TODO:
     - Handling of hyperparameters
     - More efficient resizing/reconstruction
     - Use hierarchical surrogate eval modes
  */

  // TODO? Make a struct?
  const RealVector initial_point(Teuchos::Copy, 
      				 mcmcModel.continuous_variables().values(), 
				 mcmcModel.continuous_variables().length());
  int random_seed = randomSeed;  // locally incremented
  int num_exp;
  int max_hifi = (maxHifiEvals >= 0) ? maxHifiEvals : numCandidates;
  int num_hifi = 0;
  int num_it = 1;
  bool stop_metric = false;
  double prev_MI = std::numeric_limits<Real>::infinity();

  // We assume the hifiModel's active variables are the config vars
  int num_design_vars =
    hifiModel.cv() + hifiModel.div() + hifiModel.dsv() + hifiModel.drv();
  VariablesArray design_matrix, optimal_config_matrix;
  size_and_fill(hifiModel.current_variables().shared_data(), numCandidates,
		design_matrix);
  size_and_fill(hifiModel.current_variables().shared_data(), batchEvals,
		optimal_config_matrix);

  std::ofstream out_file("experimental_design_output.txt");
  RealMatrix mi_chain; // filtered chain used to computed mutual information
  RealMatrix resp_matrix; // array that will contain new hifi model evals 
  RealVector MI_vec(batchEvals); // mutual information for design point batches

  // Get initial set of hifi model evaluations
  add_lhs_hifi_data();
  num_exp = expData.num_experiments();
  apply_hifi_sim_error(random_seed, num_exp);
 
  if (outputLevel >= DEBUG_OUTPUT)
    for (size_t i=0; i<initHifiSamples; i++) // TNP ? Why is this not num_exp?
      Cout << "Exp Data  i " << i << " value = " << expData.all_data(i);

  build_designs(design_matrix);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Design Matrix   " << design_matrix << '\n';
    Cout << "Max high-fidelity model runs = " << max_hifi << "\n\n";
  }

  const ShortShortPair& orig_view = iteratedModel.current_variables().view();
  while (!stop_metric) {
    
    eval_hi2lo_stop(stop_metric, prev_MI, MI_vec, 
		    num_hifi, max_hifi, design_matrix.size());
    
    // TODO: Make function update_calibration_data() or something
    residualModel.assign_rep(std::make_shared<DataTransformModel>
			     (mcmcModel, expData, orig_view, numHyperparams,
			      obsErrorMultiplierMode, mcmcDerivOrder));
    construct_map_model();
    construct_map_optimizer(); 

    // BMA TODO: this doesn't permit use of hyperparameters (see main ctor)
    mcmcModel.continuous_variables(initial_point);
    // TNP TODO: expose opt_for_map() and run_chain() 
    calibrate();

    print_hi2lo_chain_moments();

    // Compute batch points, either because the stop metric is still false, 
    // or because the user specified no hifi evaluations. This way they
    // are reported what evaluations would be done.
    if (!stop_metric || max_hifi == 0) {

      print_hi2lo_begin(num_it);

      // Removing burn-in and thinning
      filter_chain(acceptanceChain, mi_chain, 5000);

      // Build optimal observations matrix, contains obsverations from
      // previously selected optimal designs
      //RealMatrix optimal_obs; // TNP ? This doesn't seem to be doing anything
 
      // After calibration is run, get the posterior values of the samples; 
      // loop through designs and pick batch with maximum mutual information
      choose_batch_from_mutual_info( random_seed, num_it, max_hifi, num_hifi, 
          mi_chain, design_matrix, optimal_config_matrix, MI_vec );

      if (max_hifi > 0) {
        // TODO: Should be using new batch evaluators for this.
        run_hifi(optimal_config_matrix, resp_matrix);
        apply_hifi_sim_error( random_seed, optimal_config_matrix.size(), 
            num_exp+num_hifi);
	      num_hifi += optimal_config_matrix.size();
      }
      num_it++;

      print_hi2lo_selected(num_it, optimal_config_matrix, MI_vec);
      print_hi2lo_file(out_file, num_it, optimal_config_matrix, 
          MI_vec, resp_matrix); 
    } // end MI loop
  } // end while loop
}

void NonDBayesCalibration::eval_hi2lo_stop(bool& stop_metric, double& prev_MI,
              const RealVector& MI_vec, int num_hifi, int max_hifi, 
              int num_candidates)
{
  
  // check relative MI change
  double max_MI = MI_vec[ MI_vec.length()-1 ];
  if ( prev_MI != std::numeric_limits<Real>::infinity() ){
    double MIrel = fabs((prev_MI - max_MI)/prev_MI);
    if (MIrel < 0.05) {
      stop_metric = true;
      Cout << "Experimental Design Stop Criteria met: "
           << "Relative change in mutual information is \n"
           << "sufficiently small \n" 
           << '\n';
    }
    else
      prev_MI = max_MI;
  }
  
  // check remaining number of candidates
  if (num_candidates == 0) {
    stop_metric = true;
    Cout << "Experimental Design Stop Criteria met: "
         << "Design candidates have been exhausted \n" 
         << '\n';
  }

  // check number of hifi evaluations
  if (num_hifi == max_hifi) {
    stop_metric = true;
    Cout << "Experimental Design Stop Criteria met: "
         << "Maximum number of hifi evaluations has \n"
         << "been reached \n" 
         << '\n';
  }
}

void NonDBayesCalibration::print_hi2lo_begin(int num_it)
{
  if (outputLevel >= NORMAL_OUTPUT) {
  Cout << "\n----------------------------------------------\n";
  Cout << "Begin Experimental Design Iteration " << num_it;
  Cout << "\n----------------------------------------------\n";
  }
}
	    
void NonDBayesCalibration::print_hi2lo_status(int num_it, int i, 
    			   const Variables& xi_i, double MI)
{
  Cout << "\n----------------------------------------------\n";
  Cout << "Experimental Design Iteration "<< num_it <<" Progress";
  Cout << "\n----------------------------------------------\n";
  Cout << "Design candidate " << i << " :\n";
  xi_i.write(Cout, ACTIVE_VARS);
  Cout << "Mutual Information = " << MI << '\n'; 
}
	    
void NonDBayesCalibration::print_hi2lo_chain_moments()
{
  if (outputLevel < DEBUG_OUTPUT) 
    return;

  StringArray combined_labels;
  copy_data(residualModel.continuous_variable_labels(), 
   	        combined_labels);
  NonDSampling::print_moments(Cout, chainStats, RealMatrix(), 
  "posterior variable", Pecos::STANDARD_MOMENTS, combined_labels, false); 
  // Print response moments
  StringArray resp_labels = mcmcModel.current_response().function_labels();
  NonDSampling::print_moments(Cout, fnStats, RealMatrix(), 
      "response function", Pecos::STANDARD_MOMENTS, resp_labels, false); 
}

void NonDBayesCalibration::print_hi2lo_batch_status(int num_it, int batch_n, 
    			   int batchEvals, const Variables& optimal_config, 
			   double max_MI)
{
  Cout << "\n----------------------------------------------\n";
  Cout << "Experimental Design Iteration "<< num_it <<" Progress";
  Cout << "\n----------------------------------------------\n";
  Cout << "Point " << batch_n << " of " << batchEvals 
       << " selected\n";
  Cout << "Optimal design:\n";
  optimal_config.write(Cout, ACTIVE_VARS);
  Cout << "Mutual information = " << max_MI << '\n';
  Cout << "\n";
}
 
void NonDBayesCalibration::print_hi2lo_selected(int num_it, 
    			   const VariablesArray& optimal_config_matrix, 
             const RealVector& MI_vec)
{
  if (outputLevel < NORMAL_OUTPUT) 
    return; 

  int batch_evals = optimal_config_matrix.size();
  Cout << "\n----------------------------------------------\n";
  Cout << "Experimental Design Iteration " << num_it-1 << " Complete";
  Cout << "\n----------------------------------------------\n";
  if (batch_evals > 1)
    Cout << batch_evals << " optimal designs selected\n";
  else 
    Cout << "Optimal design:\n";
  
  for (int batch_n = 0; batch_n < batch_evals; batch_n++) {
    const Variables& col = optimal_config_matrix[batch_n];

    //Cout << "Design point " << col; 
    col.write(Cout, ACTIVE_VARS); 
    // TNP NOTE: This was printing just one MI even if there was
    // a batch. I had it print for each element in the vector.
  }
  Cout << "Mutual information = " << MI_vec[MI_vec.length()-1] << '\n';
  Cout << "\n";
}

void NonDBayesCalibration::print_hi2lo_file(std::ostream& out_file, int num_it, 
    			   const VariablesArray& optimal_config_matrix, 
			        const RealVector& MI_vec, RealMatrix& resp_matrix )
{  

  int batch_evals = optimal_config_matrix.size();
  // TNP: This output spec is just so it is compatible with the
  // old way
  out_file << "ITERATION " << num_it -1 << "\n";
  if (batch_evals > 1) 
    out_file << batch_evals << " optimal designs selected\n\n";
  else
    out_file << "Optimal Design: ";

  for (int batch_n = 0; batch_n < batch_evals; batch_n++) {
    const Variables& col = optimal_config_matrix[batch_n];
      if (batch_evals > 1){ out_file << "Design point "; }
      col.write(out_file, ACTIVE_VARS);
      out_file << "Mutual Information = " << MI_vec[batch_n] << '\n';
      if (resp_matrix.numCols() > 0) { 
        RealVector col = Teuchos::getCol(Teuchos::View, resp_matrix, 
   	                                 batch_n);
        out_file << "Hifi Response = " << col << '\n';
      }
  }
}

void NonDBayesCalibration::choose_batch_from_mutual_info( int random_seed, 
                           int num_it, int max_hifi, int num_hifi,
                           RealMatrix& mi_chain, VariablesArray& design_matrix, 
                           VariablesArray& optimal_config_matrix, 
                           RealVector& MI_vec)
{

  // Choose next optimal points to add.  Note that this optimization 
  // is done for a batch of points.  The best point is chosen based on  
  // mutual information between the model parameters and experimental data.  
  // Then, conditional on adding this point, a next best point is added 
  // based on conditional MI. 
 
  // If there are fewer designs or allowed model evaluations than the 
  // batch size, resize arrays.
  if (max_hifi != 0)  
    if (design_matrix.size() < batchEvals || 
        max_hifi - num_hifi < batchEvals){ 
      batchEvals = min(design_matrix.size(), max_hifi - num_hifi);
      size_and_fill(hifiModel.current_variables().shared_data(), batchEvals,
		    optimal_config_matrix);
      MI_vec.resize(batchEvals);
    }

  //int batch_size = MI_vec.length();
  double max_MI;
  int num_filtered = mi_chain.numCols();
  size_t optimal_ind;
  RealMatrix Xmatrix;
  // For loop for batch MI 
  for (int batch_n = 1; batch_n < batchEvals+1; batch_n ++) {
    Xmatrix.reshape(numContinuousVars + batch_n * numFunctions,
                    num_filtered);
	
    // Build simulation error matrix
    RealMatrix sim_error_matrix;
    const RealVector& sim_error_vec = mcmcModel.current_response().
                                      shared_data().simulation_error();
    if (sim_error_vec.length() > 0) {
      sim_error_matrix.reshape(numFunctions, num_filtered);
      build_error_matrix(sim_error_vec, sim_error_matrix, random_seed);
    }

    for (size_t i=0; i < design_matrix.size(); i++) {
      const Variables& xi_i = design_matrix[i]; // active are config vars
      mcmcModel.current_variables().active_to_inactive_variables(xi_i);

      build_hi2lo_xmatrix(Xmatrix, batch_n, mi_chain, sim_error_matrix);

      // calculate the mutual information b/w post theta and lofi responses
      Real MI = knn_mutual_info(Xmatrix, numContinuousVars,
			        batch_n * numFunctions, mutualInfoAlg);
      if (outputLevel >= NORMAL_OUTPUT) 
        print_hi2lo_status(num_it, i, xi_i, MI);
    
      // Now track max MI:
      if (i == 0) {
        max_MI = MI;
        optimal_ind = i;
      }
      else 
        if ( MI > max_MI) {
          max_MI = MI;
          optimal_ind = i;
        }
    } // end for over the number of candidates
    
    MI_vec[batch_n-1] = max_MI;

    // store the optimal config and a convenience reference (active are config)
    optimal_config_matrix[batch_n-1] = design_matrix[optimal_ind].copy();
    const Variables& optimal_config = optimal_config_matrix[batch_n-1];

    // Update optimal_obs matrix
    if (batchEvals > 1) {
    // Evaluate lofi model at optimal design, update Xmatrix
      RealMatrix lofi_resp_matrix;
      mcmcModel.current_variables().active_to_inactive_variables(optimal_config);
      Model::evaluate(mi_chain, mcmcModel, lofi_resp_matrix);
      if (sim_error_matrix.numRows() > 0)
        lofi_resp_matrix += sim_error_matrix;

      // TNP ? What is this for? 
      // BMA: looks like it's saving the optimal responses
      // corresponding to optimal config var values?
      //RealMatrix optimal_obs
      //  (Teuchos::View, Xmatrix, numFunctions, num_filtered,
      //   numContinuousVars + (batch_n-1)*numFunctions, 0);
      //optimal_obs.assign(lofi_resp_matrix);
    }

    // update list of candidates
    design_matrix.erase(design_matrix.begin() + optimal_ind);
    //--num_candidates;
    if (batchEvals > 1) 
      if (outputLevel >= NORMAL_OUTPUT) 
        print_hi2lo_batch_status(num_it, batch_n, batchEvals, 
				 optimal_config, max_MI);
  } // end batch_n loop

}

void NonDBayesCalibration::add_lhs_hifi_data()
{
  // If #init hifi samples, do a no-op 
  if ( initHifiSamples <= expData.num_experiments() )
    return;

  hifiSampler.run();

  int num_exp = expData.num_experiments();
  const VariablesArray& all_variables = hifiSampler.all_variables();
  const IntResponseMap& all_responses = hifiSampler.all_responses();

  if (num_exp == 0) {
    // No file data; all initial hifi calibration data points come from LHS
    // BMA TODO: Once ExperimentData can be updated, post this into
    // expData directly
    ExperimentData exp_data(initHifiSamples,
			    mcmcModel.current_variables().shared_data(),
                            mcmcModel.current_response().shared_data(), 
                            all_variables, all_responses, outputLevel);
    expData = exp_data;
  }
  else {
    // If number of num_experiments is less than the number of desired initial
    // samples, run LHS to supplement
    IntRespMCIter responses_it = all_responses.begin();
    IntRespMCIter responses_end = all_responses.end();
    for (int i=0 ; responses_it != responses_end; ++responses_it, ++i) {
      expData.add_data(mcmcModel.current_variables().shared_data(),
		       all_variables[i], responses_it->second.copy());
    }
  }
}

void NonDBayesCalibration::apply_hifi_sim_error(int& random_seed, 
    int num_exp, int exp_offset){
  // Apply hifi error
  const RealVector& hifi_sim_error = hifiModel.current_response().
                                       shared_data().simulation_error();
  if (hifi_sim_error.length() > 0){
    for (int i = 0; i < num_exp; i++) 
      apply_error_vec(hifi_sim_error, random_seed, exp_offset+i);
  }
}

void NonDBayesCalibration::apply_error_vec(const RealVector& sim_error_vec,
    			   int &stoch_seed, int experiment)
{ 
  // TNP TODO: Seems like tests don't cover sim err length > 1
  //int num_exp = expData.num_experiments();
  RealVector error_vec(numFunctions);
  Real stdev;
  boost::mt19937 rnumGenerator;
  if (sim_error_vec.length() == 1) {
    rnumGenerator.seed(stoch_seed);
    stdev = std::sqrt(sim_error_vec[0]);
    boost::normal_distribution<> err_dist(0.0, stdev);
    boost::variate_generator<boost::mt19937,
           boost::normal_distribution<> > err_gen(rnumGenerator, err_dist);
      for (size_t j = 0; j < numFunctions; j++)
        error_vec[j] = err_gen();
      expData.apply_simulation_error(error_vec, experiment);
  }
  else {
      for (size_t j = 0; j < numFunctions; j++) {
        ++stoch_seed;
        stdev = std::sqrt(sim_error_vec[j]);
        rnumGenerator.seed(stoch_seed);
        boost::normal_distribution<> err_dist(0.0, stdev);
        boost::variate_generator<boost::mt19937,
               boost::normal_distribution<> > err_gen(rnumGenerator, err_dist);
        error_vec[j] = err_gen();
      }
      expData.apply_simulation_error(error_vec, experiment);
  }
  ++stoch_seed;
}

void NonDBayesCalibration::build_error_matrix(const RealVector& sim_error_vec,
                           RealMatrix& sim_error_matrix, int &stoch_seed)
{
  Real stdev;
  RealVector col_vec(numFunctions);
  boost::mt19937 rnumGenerator;
  int num_filtered = sim_error_matrix.numCols();
  ++stoch_seed;
  if (sim_error_vec.length() == 1) {
    rnumGenerator.seed(stoch_seed);
    stdev = std::sqrt(sim_error_vec[0]);
    boost::normal_distribution<> err_dist(0.0, stdev);
    boost::variate_generator<boost::mt19937, 
                             boost::normal_distribution<> >
           err_gen(rnumGenerator, err_dist);
    for (int j = 0; j < num_filtered; j++) {
      for (size_t k = 0; k < numFunctions; k++) {
        col_vec[k] = err_gen();
      }
      Teuchos::setCol(col_vec, j, sim_error_matrix);
    }
  }
  else {
    for (int j = 0; j < num_filtered; j++) {
      for (size_t k = 0; k < numFunctions; k++) {
        ++stoch_seed;
        rnumGenerator.seed(stoch_seed);
        stdev = std::sqrt(sim_error_vec[k]);
        boost::normal_distribution<> err_dist(0.0, stdev);
        boost::variate_generator<boost::mt19937,
                   boost::normal_distribution<> >
               err_gen(rnumGenerator, err_dist);
        col_vec[k] = err_gen();
      }
      Teuchos::setCol(col_vec, j, sim_error_matrix);
    }
  }
}

/** On entry, design_matrix already sized to numCandidates. */
void NonDBayesCalibration::build_designs(VariablesArray& design_matrix)
{
  // We assume the hifiModel's active variables are the config vars
  size_t num_candidates_in = 0;

  // If available, import data first into beginning of design_matrix
  if (!importCandPtsFile.empty()) {

    // BMA TODO: maybe merge read config vars single file?
    // read directly into design_matrix, until at most numCandidates
    auto numin_havemore = TabularIO::read_data_tabular
      (importCandPtsFile, "user-provided candidate points",
       numCandidates, design_matrix, importCandFormat);

    num_candidates_in = numin_havemore.first;
    if (numin_havemore.second && outputLevel >= VERBOSE_OUTPUT) {
      Cout << "\nWarning: Bayesian design of experiments only using the "
	   << "first " << numCandidates << " candidates in " 
	   << importCandPtsFile << '\n';
    }
  }

  // If needed, supplement with LHS-generated designs
  if (num_candidates_in < numCandidates) {
    size_t new_candidates = numCandidates - num_candidates_in;

    Iterator lhs_iterator2;
    unsigned short sample_type = SUBMETHOD_LHS;
    bool vary_pattern = true;
    String rng("mt19937");
    int random_seed_1 = randomSeed+1;
    auto lhs_sampler_rep2 = std::make_shared<NonDLHSSampling>
      (hifiModel, sample_type, new_candidates, random_seed_1,
       rng, vary_pattern, ACTIVE_UNIFORM);
    lhs_iterator2.assign_rep(lhs_sampler_rep2);
    lhs_iterator2.pre_run();
    // populate the sub-set (possibly full set) of generated candidates
    // could skip this copy if the lhs_iterator2 doesn't share any other uses
    const auto& lhs_all_vars = lhs_iterator2.all_variables();
    for (size_t i=0; i<new_candidates; ++i)
      design_matrix[num_candidates_in + i] = lhs_all_vars[i].copy();
  }
}
	  
void NonDBayesCalibration::build_hi2lo_xmatrix(RealMatrix& Xmatrix, int i, 
    			   const RealMatrix& mi_chain, RealMatrix& 
			   sim_error_matrix)
{
  // run the model at each posterior sample, with option to batch
  // evaluate the lo-fi model
 
  // receive the evals in a separate matrix for safety
  RealMatrix lofi_resp_matrix;
  // BMA: I believe mi_chain should be the active calibration
  // parameters theta only, so the mcmcModel (DataTransformModel)
  // manages the config vars...
  Model::evaluate(mi_chain, mcmcModel, lofi_resp_matrix);
 
  //concatenate posterior_theta and lofi_resp_mat into Xmatrix
  RealMatrix xmatrix_theta(Teuchos::View, Xmatrix,
 		   numContinuousVars, Xmatrix.numCols());
  xmatrix_theta.assign(mi_chain);
 
  RealMatrix xmatrix_curr_responses
    (Teuchos::View, Xmatrix, numFunctions, Xmatrix.numCols(),
     numContinuousVars + (i-1)*numFunctions, 0);
  xmatrix_curr_responses.assign(lofi_resp_matrix);
  
  if (sim_error_matrix.numRows() > 0)
    xmatrix_curr_responses += sim_error_matrix;
}

void 
NonDBayesCalibration::run_hifi(const VariablesArray& optimal_config_matrix, 
			       RealMatrix& resp_matrix)
{
  // batch evaluate hifiModel, populating resp_matrix
  // evaluate sends the passed Variables to active on hifiModel
  Model::evaluate(optimal_config_matrix, hifiModel, resp_matrix);
  // update hifi experiment data
  RealMatrix::ordinalType col_ind;
  RealMatrix::ordinalType num_evals = optimal_config_matrix.size();
  for (col_ind = 0; col_ind < num_evals; ++col_ind) {
    
    const Variables& config_vars = optimal_config_matrix[col_ind];

    // ExperimentData requires a new Response for each insertion
    RealVector hifi_fn_vals =
      Teuchos::getCol(Teuchos::Copy, resp_matrix, col_ind);
    Response hifi_resp = hifiModel.current_response().copy();
    hifi_resp.function_values(hifi_fn_vals);

    expData.add_data(mcmcModel.current_variables().shared_data(),
		     config_vars, hifi_resp);
  }
}
    
void NonDBayesCalibration::build_model_discrepancy()
{
  size_t num_field_groups = expData.num_fields();
  if (num_field_groups == 0)
    build_scalar_discrepancy();
  else {
    if (readFieldCoords)
      build_field_discrepancy();
    else {
      Cout << "You must specify read_field_coodinates in input file in order "
           << "to calculate model discrepancy\n";
      abort_handler(METHOD_ERROR);
    }
  }
}

void NonDBayesCalibration::build_scalar_discrepancy()
{
  // For now, use average params (unfiltered)
  RealMatrix acc_chain_transpose(acceptanceChain, Teuchos::TRANS);
  int num_cols = acc_chain_transpose.numCols();
  RealVector ave_params(num_cols);
  compute_col_means(acc_chain_transpose, ave_params); 
  mcmcModel.continuous_variables(ave_params);
  
  int num_exp = expData.num_experiments();
  size_t num_configvars = expData.num_config_vars();
  std::vector<RealVector> config_vars = expData.config_vars_as_real();
  RealMatrix allConfigInputs(num_configvars,num_exp);
  for (int i = 0; i < num_exp; i++) {
    const RealVector& config_vec = config_vars[i];
    Teuchos::setCol(config_vec, i, allConfigInputs);
  } 

  // Initialize DiscrepancyCorrection class
  SizetSet fn_indices;
  // Hardcode for now, call id_surrogates eventually? See SurrogateModel.cpp 
  for (size_t i = 0; i < numFunctions; ++i)
    fn_indices.insert(i);
  DiscrepancyCorrection modelDisc;
  short corr_type = ADDITIVE_CORRECTION, corr_order = 0;
  modelDisc.initialize(fn_indices, numFunctions, num_configvars, corr_type, 
       		       corr_order, discrepancyType, discrepPolyOrder);

  // Construct config var information
  Variables vars_copy = mcmcModel.current_variables().copy();
  ShortShortPair view(MIXED_STATE, EMPTY_VIEW);
  SizetArray vars_comps_totals(NUM_VC_TOTALS, 0);
  vars_comps_totals = mcmcModel.current_variables().shared_data().
    		      inactive_components_totals();
  SharedVariablesData svd(view, vars_comps_totals);
  Variables configvars(svd);
  VariablesArray configvar_array(num_exp);
  for (int i=0; i<num_exp; i++) {
    const RealVector& config_i = Teuchos::getCol(Teuchos::View, 
				 allConfigInputs, i);
    Model::inactive_variables(config_i, mcmcModel, vars_copy);
    configvars.continuous_variables(vars_copy.inactive_continuous_variables());
    configvars.discrete_int_variables(vars_copy.
				      inactive_discrete_int_variables());
    configvars.discrete_real_variables(vars_copy.
				       inactive_discrete_real_variables());
    configvar_array[i] = configvars.copy();
  }

  // Construct response information from expData and model 
  ResponseArray simresponse_array(num_exp);
  ResponseArray expresponse_array(num_exp);
  for (int i = 0; i<num_exp; i++){
    RealVector config_vec = Teuchos::getCol(Teuchos::View, allConfigInputs, 
		 	    i);
    Model::inactive_variables(config_vec, mcmcModel);
    mcmcModel.evaluate();
    simresponse_array[i] = mcmcModel.current_response().copy();
    expresponse_array[i] = expData.response(i);
  }
  //Cout << "sim response array = " << simresponse_array << '\n';
  //Cout << "exp response array = " << expresponse_array << '\n';
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  modelDisc.compute(configvar_array, expresponse_array, simresponse_array, 
      		    quiet_flag);
  
  // Construct config var information for prediction configs   
  int num_pred;
  RealMatrix configpred_mat; 
  RealVector config(1); //KAM: currently assume only one config var
  VariablesArray configpred_array;
  if (!importPredConfigs.empty()) {
    TabularIO::read_data_tabular(importPredConfigs,
				 "user-provided prediction configurations",
				 configpred_mat, num_configvars, 
				 importPredConfigFormat, false);
    num_pred = configpred_mat.numCols();
    configpred_array.resize(num_pred);
    for (int i = 0; i < num_pred; i++) {
      config = Teuchos::getCol(Teuchos::View, configpred_mat, i);
      configvars.continuous_variables(config);
      configpred_array[i] = configvars.copy();
    } 

  }
  else if (!predictionConfigList.empty()) {
    num_pred = predictionConfigList.length(); 
    configpred_array.resize(num_pred);
    configpred_mat.shapeUninitialized(num_configvars, num_pred);
    for (int i = 0; i < num_pred; i++) {
      config = predictionConfigList[i];
      configvars.continuous_variables(config);
      configpred_array[i] = configvars.copy();
      Teuchos::setCol(config, i, configpred_mat);
    } 
  }
  else {
    num_pred = ( numPredConfigs > 0) ? numPredConfigs : 10;
    configpred_array.resize(num_pred);
    configpred_mat.shapeUninitialized(num_configvars, num_pred);
    double config_step;
    if (numPredConfigs == 1)
      config_step = (configUpperBnds[0] - configLowerBnds[0])/2;
    else
      config_step = (configUpperBnds[0]-configLowerBnds[0])/(num_pred-1);
    for (int i = 0; i < num_pred; i++){
      config = configLowerBnds[0] + config_step*i;
      configvars.continuous_variables(config);
      configpred_array[i] = configvars.copy();
      Teuchos::setCol(config, i, configpred_mat);
    }
  }

  // Compute dsicrepancy approx and corrected response
  correctedResponses.resize(num_pred);
  discrepancyResponses.resize(num_pred);
  Response zero_response = mcmcModel.current_response().copy();
  for (int i = 0; i < num_pred; i++) {
    for (size_t j = 0; j < numFunctions; j++) 
      zero_response.function_value(0,j);
    RealVector config_vec = Teuchos::getCol(Teuchos::View, configpred_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    mcmcModel.continuous_variables(ave_params); //KAM -delete later
    mcmcModel.evaluate();
    Variables configpred = configpred_array[i];
    Response simresponse_pred = mcmcModel.current_response();
    Cout << "Calculating model discrepancy";
    modelDisc.apply(configpred, zero_response, quiet_flag);
    discrepancyResponses[i] = zero_response.copy();
    Cout << "Correcting model response";
    modelDisc.apply(configpred, simresponse_pred, quiet_flag);
    correctedResponses[i] = simresponse_pred.copy();
  } 
  
  // Compute correction variance 
  RealMatrix discrep_var(num_pred, numFunctions);
  correctedVariances.shapeUninitialized(num_pred, numFunctions);
  modelDisc.compute_variance(configpred_array, discrep_var, quiet_flag);
  if (expData.variance_active()) {
    RealVectorArray exp_stddevs(num_exp*numFunctions);
    expData.cov_std_deviation(exp_stddevs); // one vector per experiment
    RealVector col_vec(num_pred);
    for (int i = 0; i < numFunctions; i++) {
      Real& max_var = exp_stddevs[0][i];
      for (int j = 0; j < num_exp; j++) 
       if (exp_stddevs[j][i] > max_var)
	 max_var = exp_stddevs[j][i];
      RealVector discrep_varvec = Teuchos::getCol(Teuchos::View, discrep_var,i);
      for (int j = 0; j < num_pred; j++) {
    	col_vec[j] = discrep_varvec[j] + pow(max_var, 2);
      }
      Teuchos::setCol(col_vec, i, correctedVariances);
    }
  }
  else {
    correctedVariances = discrep_var;
    Cout << "\nWarning: No variance information was provided in " 
         << scalarDataFilename << ".\n         Prediction variance computed "
	 << "contains only variance information\n         from the " 
	 << "discrepancy model.\n";
  }
  
  export_discrepancy(configpred_mat);
}

void NonDBayesCalibration::build_field_discrepancy()
{
  // For now, use average params (unfiltered)
  RealMatrix acc_chain_transpose(acceptanceChain, Teuchos::TRANS);
  int num_cols = acc_chain_transpose.numCols();
  RealVector ave_params(num_cols);
  compute_col_means(acc_chain_transpose, ave_params); 
  mcmcModel.continuous_variables(ave_params);
  //mcmcModel.evaluate();

  int num_exp = expData.num_experiments();
  size_t num_configvars = expData.num_config_vars();
  std::vector<RealVector> config_vars = expData.config_vars_as_real();
  // KAM TODO: add catch when num_config_vars == 0. Trying to retrieve
  // this length will seg fault

  size_t num_field_groups = expData.num_fields();
  // KAM TODO: Throw error if num_field_groups > 1 (either need separate
  // discrepancy for each or only allow one field group)
  // Read independent coordinates

  // Read variables for discrepancy build points
  RealMatrix allvars_mat;
  RealVector col_vec;
  for (int i = 0; i < num_field_groups; i++) { 
    for (int j = 0; j < num_exp; j++) {
      RealMatrix vars_mat = expData.field_coords_view(i,j);
      int num_indepvars = vars_mat.numRows();
      int dim_indepvars = vars_mat.numCols();
      vars_mat.reshape(num_indepvars, dim_indepvars + num_configvars);
      const RealVector& config_vec = config_vars[j];
      col_vec.resize(num_indepvars);
      for (int k = 0; k < num_configvars; k++) {
        col_vec.putScalar(config_vec[k]);
        Teuchos::setCol(col_vec, dim_indepvars+k, vars_mat);
      }
      // add to total matrix
      RealMatrix varsmat_trans(vars_mat, Teuchos::TRANS);
      int col = allvars_mat.numCols();
      allvars_mat.reshape(vars_mat.numCols(), 
                          allvars_mat.numCols() + num_indepvars);
      for (int k = 0; k < varsmat_trans.numCols(); k ++) {
        col_vec = Teuchos::getCol(Teuchos::Copy, varsmat_trans, k);
        Teuchos::setCol(col_vec, col+k, allvars_mat);
      }
    }
  }

  /*
  ShortArray my_asv(3);
  my_asv[0]=1;
  my_asv[1]=0;
  my_asv[2]=0;
  */
  ShortArray exp_asv(num_exp);
  for (int i = 0; i < num_exp; i++) {
    exp_asv[i] = 1;
  }

  // Build concatenated vector for discrepancy
  //for (int i = 0; i < num_field_groups; i++) {
  int ind = 0;
  RealVector concat_disc;
  for (int i = 0; i < num_exp; i++) {
    const IntVector field_lengths = expData.field_lengths(i);
    const RealVector& config_vec = config_vars[i];
    Model::inactive_variables(config_vec, mcmcModel);
    mcmcModel.evaluate();
    for (int j = 0; j < field_lengths.length(); j++) {
      concat_disc.resize(ind + field_lengths[j]);
      if (expData.interpolate_flag()) {
        const Response model_resp = mcmcModel.current_response().copy();
        Response interpolated_resp = expData.response(i).copy();
        expData.interpolate_simulation_data(model_resp, i, exp_asv, 0, 
                                            interpolated_resp);
        for (int k=0; k<field_lengths[j]; k++)
          concat_disc[ind + k] = expData.all_data(i)[k] - 
                                 interpolated_resp.function_values()[k];
      }
      else {
        for (int k=0; k<field_lengths[j]; k++)
          concat_disc[ind + k] = expData.all_data(i)[k] - 
                              mcmcModel.current_response().function_values()[k];
      }
      ind += field_lengths[j];
    }
  }
  //Cout << "disc_build_points " <<concat_disc;

  // Build matrix containing prediction points for discrepancy
  RealMatrix discrepvars_pred;
  RealMatrix configpred_mat;
  int num_pred;
  // Read in prediction config vars
  if (!importPredConfigs.empty()) {
    TabularIO::read_data_tabular(importPredConfigs,
                                 "user-provided prediction configurations",
                                 configpred_mat, num_configvars,
                                 importPredConfigFormat, false);
    num_pred = configpred_mat.numCols();
  }
  else if (!predictionConfigList.empty()) {
    //KAM TODO: check length % num_config vars
    num_pred = predictionConfigList.length()/num_configvars;
    configpred_mat.shapeUninitialized(num_configvars, num_pred);
    RealVector config(num_configvars);
    for (int j = 0; j < num_pred; j++) {
      for (int k = 0; k < num_configvars; k++) 
        config[k] = predictionConfigList[j*num_configvars + k];
      Teuchos::setCol(config, j, configpred_mat);
    }
  }
  else if (numPredConfigs > 1) {
    num_pred = numPredConfigs;
    configpred_mat.shapeUninitialized(num_configvars, numPredConfigs);
    RealVector config_step(num_configvars);
    RealVector config(num_configvars);
    for (int i = 0; i < num_configvars; i++)
      config_step[i] = (configUpperBnds[i] -
                              configLowerBnds[i])/(numPredConfigs - 1);
    for (int i = 0; i < numPredConfigs; i++) {
      for (int j = 0; j < num_configvars; j++)
        config[j] = configLowerBnds[j] + config_step[j]*i;
      Teuchos::setCol(config, i, configpred_mat);
    }
  }
  else {
    for (int j = 0; j < num_exp; j++) {
      num_pred = num_exp;
      configpred_mat.shapeUninitialized(num_configvars, num_exp);
      const RealVector& config_vec = config_vars[j];
      Teuchos::setCol(config_vec, j, configpred_mat);
    }
  }
  //Cout << "pred config mat = " << configpred_mat << '\n';
  // Combine with simulation indep vars
  for (int i = 0; i < num_field_groups; i ++) {
    for (int j = 0; j < num_pred; j++) {
      RealMatrix vars_mat = mcmcModel.current_response().field_coords_view(i);
      int num_indepvars = vars_mat.numRows();
      int dim_indepvars = vars_mat.numCols();
      vars_mat.reshape(num_indepvars, dim_indepvars + num_configvars);
      const RealVector& config_vec = Teuchos::getCol(Teuchos::Copy,
                                              configpred_mat, j);
      col_vec.resize(num_indepvars);
      for (int k = 0; k < num_configvars; k++) {
        col_vec.putScalar(config_vec[k]);
        Teuchos::setCol(col_vec, dim_indepvars+k, vars_mat);
      }
      RealMatrix varsmat_trans(vars_mat, Teuchos::TRANS);
      int col = discrepvars_pred.numCols();
      discrepvars_pred.reshape(vars_mat.numCols(), 
                          discrepvars_pred.numCols() + num_indepvars);
      for (int k = 0; k < varsmat_trans.numCols(); k ++) {
        col_vec = Teuchos::getCol(Teuchos::Copy, varsmat_trans, k);
        Teuchos::setCol(col_vec, col+k, discrepvars_pred);
      }
    }
  }

  //Cout << "All vars matrix " << allvars_mat << '\n';
  //Cout << "Pred vars matrix " << discrepvars_pred << '\n';
  discrepancyFieldResponses.resize(discrepvars_pred.numCols());
  correctedFieldVariances.resize(discrepvars_pred.numCols());
  build_GP_field(allvars_mat, discrepvars_pred, concat_disc, 
                 discrepancyFieldResponses, correctedFieldVariances);
  //Cout << "disc_pred " << discrepancyFieldResponses;
  //Cout << "disc_var = " << correctedFieldVariances << '\n';
  //TODO: Currently, correctedFieldVariances contains only variance from
  //the discrepancy approx. This needs to be combined with the experimental
  //variance 

  // Compute corrected response
  ind = 0;
  correctedFieldResponses.resize(discrepvars_pred.numCols());
  for (int i = 0; i < num_pred; i++) {
    const RealVector& config_vec = Teuchos::getCol(Teuchos::Copy,
                                            configpred_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    mcmcModel.evaluate();
    const RealVector sim_resp = mcmcModel.current_response().function_values();
    for (int j = 0; j < sim_resp.length(); j ++) { 
      correctedFieldResponses[ind + j] = sim_resp[j] 
                              + discrepancyFieldResponses[ind+j];
    }
    ind += sim_resp.length();
  }

    //allExperiments[exp_ind].function_value(i);
    //Response residual_response;
    //expData.form_residuals(mcmcModel.current_response(), j, my_asv, 0, residual_response);
    //expData.form_residuals(mcmcModel.current_response(), j, residual_response);
    //build_GP_field(vector view(indep_coordinates, t_new_coord, concat_disc, disc_pred);
    //Cout << residual_response.function_values();
  //}

  // We are getting coordinates for the experiment and ultimately we need to handle the simulation coordinates
  // if interpolation.
  //
  // Initialize newFnDiscClass
  // newFnDiscClass.initialize(x, theta, t) 
  // newFnDiscClass.build(vector t, vector responses, correction_term) 
  // newFnDiscClass.apply(vector t_new, vector responses, corrected_responses) 
  // One possibility is to use the GP construction from NonDLHSSampling, line 794
  // Another is to use the DiscrepancyCorrection class 
  
  export_field_discrepancy(discrepvars_pred);
}

void NonDBayesCalibration::build_GP_field(const RealMatrix& discrep_vars_mat, 
                           RealMatrix& discrep_vars_pred,
			   const RealVector& concat_disc, RealVector& disc_pred,
                           RealVector& disc_var)
{
  String approx_type;
  approx_type = "global_kriging";  // Surfpack GP
  UShortArray approx_order;
  short data_order = 1;  // assume only function values
  short output_level = NORMAL_OUTPUT;
  SharedApproxData sharedData;
  // if multiple independent coordinates and config vars, need to 
  // change 1 to number of input parameters for GP
  int num_GP_inputs = discrep_vars_mat.numRows();
  sharedData = SharedApproxData(approx_type, approx_order, num_GP_inputs,
                                data_order, output_level);
  Approximation gpApproximation(sharedData);

  // build the GP for the discrepancy
  gpApproximation.add_array(discrep_vars_mat, true, concat_disc, true); // deep
  gpApproximation.build();
  //gpApproximations.export_model(GPstring, GPPrefix, ALGEBRAIC_FILE);
  int pred_length = discrep_vars_pred.numCols();
  for (int i=0; i<pred_length; i++) {
    RealVector new_sample = Teuchos::getCol(Teuchos::View,discrep_vars_pred,i);
    disc_pred(i)=gpApproximation.value(new_sample);
    disc_var(i) = gpApproximation.prediction_variance(new_sample);
  }
}

void NonDBayesCalibration::export_discrepancy(RealMatrix& 
    			   pred_config_mat)
{

  // Calculate number of predictions
  int num_pred = pred_config_mat.numCols();
  Variables output_vars = mcmcModel.current_variables().copy(); 
  const StringArray& resp_labels = 
    		     mcmcModel.current_response().function_labels();
  size_t wpp4 = write_precision+4;

  // Discrepancy responses file output
  unsigned short discrep_format = exportDiscrepFormat;
  String discrep_filename = 
    exportDiscrepFile.empty() ? "dakota_discrepancy_tabular.dat" : 
    exportDiscrepFile;
  std::ofstream discrep_stream;
  TabularIO::open_file(discrep_stream, discrep_filename, 
      		       "NonDBayesCalibration discrepancy response export");

  TabularIO::write_header_tabular(discrep_stream, output_vars, resp_labels, 
      				  "config_id", "interface", discrep_format);
  discrep_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(discrep_stream, i+1, 
				     mcmcModel.interface_id(), 
				     discrep_format);
    const RealVector& config_vec = Teuchos::getCol(Teuchos::View, 
						   pred_config_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    output_vars = mcmcModel.current_variables().copy();
    output_vars.write_tabular(discrep_stream);
    const RealVector& resp_vec = discrepancyResponses[i].function_values();
    for (size_t j = 0; j < numFunctions; ++j) 
      discrep_stream << std::setw(wpp4) << resp_vec[j] << ' ';
    discrep_stream << '\n';
  }
  TabularIO::close_file(discrep_stream, discrep_filename, 
      	 		"NonDBayesCalibration discrepancy response export");

  // Corrected model (model+discrep) file output
  unsigned short corrmodel_format = exportCorrModelFormat;
  String corrmodel_filename = 
    exportCorrModelFile.empty() ? "dakota_corrected_model_tabular.dat" : 
    exportCorrModelFile;
  std::ofstream corrmodel_stream;
  TabularIO::open_file(corrmodel_stream, corrmodel_filename, 
      		       "NonDBayesCalibration corrected model response export");

  TabularIO::write_header_tabular(corrmodel_stream, output_vars, resp_labels, 
      				  "config_id", "interface", corrmodel_format);
  corrmodel_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(corrmodel_stream, i+1, 
				     mcmcModel.interface_id(), 
				     corrmodel_format);
    const RealVector& config_vec = Teuchos::getCol(Teuchos::View, 
						   pred_config_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    output_vars = mcmcModel.current_variables().copy();
    output_vars.write_tabular(corrmodel_stream);
    const RealVector& resp_vec = correctedResponses[i].function_values();
    for (size_t j = 0; j < numFunctions; ++j) 
      corrmodel_stream << std::setw(wpp4) << resp_vec[j] << ' ';
    corrmodel_stream << '\n';
  }
  TabularIO::close_file(corrmodel_stream, corrmodel_filename, 
      	 		"NonDBayesCalibration corrected model response export");

  // Corrected model variances file output
  unsigned short discrepvar_format = exportCorrVarFormat;
  String var_filename = exportCorrVarFile.empty() ? 
    		 "dakota_discrepancy_variance_tabular.dat" : exportCorrVarFile;
  //discrep_filename = "dakota_corrected_variances.dat";
  std::ofstream discrepvar_stream;
  TabularIO::open_file(discrepvar_stream, var_filename, 
      		       "NonDBayesCalibration corrected model variance export");

  RealMatrix corrected_var_transpose(correctedVariances, Teuchos::TRANS);
  StringArray var_labels(numFunctions);
  for (int i = 0; i < numFunctions; i++) {
    std::stringstream s;
    s << resp_labels[i] << "_var";
    var_labels[i] = s.str();
  }
  TabularIO::write_header_tabular(discrepvar_stream, output_vars, var_labels, 
      				  "pred_config", "interface",discrepvar_format);
  discrepvar_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(discrepvar_stream, i+1, 
				     mcmcModel.interface_id(), 
				     discrepvar_format);
    const RealVector& config_vec = Teuchos::getCol(Teuchos::View, 
						   pred_config_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    output_vars = mcmcModel.current_variables().copy();
    output_vars.write_tabular(discrepvar_stream);
    const RealVector& var_vec = Teuchos::getCol(Teuchos::View, 
						corrected_var_transpose, i);
    for (size_t j = 0; j < numFunctions; ++j) 
      discrepvar_stream << std::setw(wpp4) << var_vec[j] << ' ';
    discrepvar_stream << '\n';
  }
  TabularIO::close_file(discrepvar_stream, var_filename, 
      	 		"NonDBayesCalibration corrected model variance export");
}

void NonDBayesCalibration::export_field_discrepancy(RealMatrix& pred_vars_mat)
{
  // Calculate number of predictions
  int num_pred = pred_vars_mat.numCols()/numFunctions;
  size_t num_field_groups = expData.num_fields();
  Variables output_vars = mcmcModel.current_variables().copy();
  const StringArray& resp_labels = 
    		     mcmcModel.current_response().function_labels();
  size_t wpp4 = write_precision+4;

  // Discrepancy responses file output
  unsigned short discrep_format = exportDiscrepFormat;
  String discrep_filename = 
    exportDiscrepFile.empty() ? "dakota_discrepancy_tabular.dat" : 
    exportDiscrepFile;
  std::ofstream discrep_stream;
  TabularIO::open_file(discrep_stream, discrep_filename, 
      		       "NonDBayesCalibration discrepancy response export");

  TabularIO::write_header_tabular(discrep_stream, output_vars, resp_labels, 
      				  "config_id", "interface", discrep_format);
  discrep_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  int ind = 0;
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(discrep_stream, i+1, 
				     mcmcModel.interface_id(), 
				     discrep_format);
    const RealVector& pred_vec = Teuchos::getCol(Teuchos::View, 
					  pred_vars_mat, int(numFunctions)*i);
    for (int j = 0; j < num_field_groups; j++) {
      RealMatrix vars_mat = mcmcModel.current_response().field_coords_view(j);
      int field_length = vars_mat.numRows();
      int indepvars_dim = vars_mat.numCols();
      int config_length = pred_vec.length() - indepvars_dim;
      RealVector config_vec(config_length);
      for (int k = 0; k < config_length; k ++)
        config_vec[k] = pred_vec[indepvars_dim + k];
      Model::inactive_variables(config_vec, mcmcModel);
      output_vars = mcmcModel.current_variables().copy();
      output_vars.write_tabular(discrep_stream);
      for (int k = 0; k < field_length; k++) 
        discrep_stream << std::setw(wpp4) << discrepancyFieldResponses[ind + k] 
                       << ' ';
      ind += field_length;
    }
    discrep_stream << '\n';
  }
  TabularIO::close_file(discrep_stream, discrep_filename, 
      	 		"NonDBayesCalibration discrepancy response export");

  // Corrected model (model+discrep) file output
  unsigned short corrmodel_format = exportCorrModelFormat;
  String corrmodel_filename = 
    exportCorrModelFile.empty() ? "dakota_corrected_model_tabular.dat" : 
    exportCorrModelFile;
  std::ofstream corrmodel_stream;
  TabularIO::open_file(corrmodel_stream, corrmodel_filename, 
      		       "NonDBayesCalibration corrected model response export");

  TabularIO::write_header_tabular(corrmodel_stream, output_vars, resp_labels, 
      				  "config_id", "interface", corrmodel_format);
  corrmodel_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  ind = 0;
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(corrmodel_stream, i+1, 
				     mcmcModel.interface_id(), 
				     corrmodel_format);
    const RealVector& pred_vec = Teuchos::getCol(Teuchos::View, 
					  pred_vars_mat, int(numFunctions)*i);
    for (int j = 0; j < num_field_groups; j++) {
      RealMatrix vars_mat = mcmcModel.current_response().field_coords_view(j);
      int field_length = vars_mat.numRows();
      int indepvars_dim = vars_mat.numCols();
      int config_length = pred_vec.length() - indepvars_dim;
      RealVector config_vec(config_length);
      for (int k = 0; k < config_length; k ++)
        config_vec[k] = pred_vec[indepvars_dim + k];
      Model::inactive_variables(config_vec, mcmcModel);
      output_vars = mcmcModel.current_variables().copy();
      output_vars.write_tabular(corrmodel_stream);
      for (int k = 0; k < field_length; k++) 
        corrmodel_stream << std::setw(wpp4) << correctedFieldResponses[ind + k] 
                       << ' ';
      ind += field_length;
    }
    corrmodel_stream << '\n';
  }
  TabularIO::close_file(corrmodel_stream, corrmodel_filename, 
      	 		"NonDBayesCalibration corrected model response export");

  // Discrepancy variances file output 
  // TODO: Corrected model variances file output
  unsigned short discrepvar_format = exportCorrVarFormat;
  String var_filename = exportCorrVarFile.empty() ? 
    		 "dakota_discrepancy_variance_tabular.dat" : exportCorrVarFile;
  //discrep_filename = "dakota_corrected_variances.dat";
  std::ofstream discrepvar_stream;
  TabularIO::open_file(discrepvar_stream, var_filename, 
      		       "NonDBayesCalibration corrected model variance export");

  //RealMatrix corrected_var_transpose(correctedVariances, Teuchos::TRANS);
  StringArray var_labels(numFunctions);
  for (int i = 0; i < numFunctions; i++) {
    std::stringstream s;
    s << resp_labels[i] << "_var";
    var_labels[i] = s.str();
  }
  TabularIO::write_header_tabular(discrepvar_stream, output_vars, var_labels, 
      				  "pred_config", "interface",discrepvar_format);
  discrepvar_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  ind = 0;
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(discrepvar_stream, i+1, 
				     mcmcModel.interface_id(), 
				     discrepvar_format);
    const RealVector& pred_vec = Teuchos::getCol(Teuchos::View, 
					  pred_vars_mat, int(numFunctions)*i);
    for (int j = 0; j < num_field_groups; j++) {
      RealMatrix vars_mat = mcmcModel.current_response().field_coords_view(j);
      int field_length = vars_mat.numRows();
      int indepvars_dim = vars_mat.numCols();
      int config_length = pred_vec.length() - indepvars_dim;
      RealVector config_vec(config_length);
      for (int k = 0; k < config_length; k ++)
        config_vec[k] = pred_vec[indepvars_dim + k];
      Model::inactive_variables(config_vec, mcmcModel);
      output_vars = mcmcModel.current_variables().copy();
      output_vars.write_tabular(discrepvar_stream);
      for (int k = 0; k < field_length; k++) 
        discrepvar_stream << std::setw(wpp4) << correctedFieldVariances[ind + k] 
                       << ' ';
      ind += field_length;
    }
    discrepvar_stream << '\n';
  }
  TabularIO::close_file(discrepvar_stream, var_filename, 
      	 		"NonDBayesCalibration corrected model variance export");
}

void NonDBayesCalibration::
extract_selected_posterior_samples(const std::vector<int> &points_to_keep,
				   const RealMatrix &samples_for_posterior_eval,
				   const RealVector &posterior_density,
				   RealMatrix &posterior_data ) const 
{
}

void NonDBayesCalibration::
export_posterior_samples_to_file( const std::string filename,
				  const RealMatrix &posterior_data ) const
{
}




/** Calculate the log-likelihood, accounting for contributions from
    covariance and hyperparameters, as well as constant term:

      log(L) = -1/2*Nr*log(2*pi) - 1/2*log(det(Cov)) - 1/2*r'(Cov^{-1})*r

    The passed residuals must already be size-adjusted, differenced
    with any data, if present, and scaled by covariance^{-1/2}. */
Real NonDBayesCalibration::
log_likelihood(const RealVector& residuals, const RealVector& all_params)
{
  // if needed, extract the trailing hyper-parameters
  RealVector hyper_params;
  if (numHyperparams > 0)
    hyper_params = RealVector(Teuchos::View, 
                              all_params.values() + numContinuousVars, 
                              numHyperparams);

  size_t num_total_calib_terms = residuals.length();
  Real half_nr_log2pi = num_total_calib_terms * HALF_LOG_2PI;
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

  if (!standardizedSpace &&
      iteratedModel.multivariate_distribution().correlation()) { // x_dist
    Teuchos::SerialSpdDenseSolver<int, Real> corr_solver;
    RealSymMatrix prior_cov_matrix;//= ();

    Cerr << "prior_cholesky_factorization() not yet implemented for this case."
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
    // SVD index conversion is more general, but not required for current uses
    //const SharedVariablesData& svd
    //  = mcmcModel.current_variables().shared_data();
    RealVector dist_stdevs  // u_dist (decorrelated)
      = mcmcModel.multivariate_distribution().std_deviations();
    for (i=0; i<numContinuousVars; ++i)
      priorCovCholFactor(i,i) = dist_stdevs[i];
	//= dist_stdevs[svd.cv_index_to_active_index(i)];

    // for now we assume a variance when the inv gamma has infinite moments
    Real alpha;
    for (i=0; i<numHyperparams; ++i) {
      invGammaDists[i].pull_parameter(Pecos::IGA_ALPHA, alpha);
      priorCovCholFactor(numContinuousVars + i, numContinuousVars + i) =
	(alpha > 2.) ? invGammaDists[i].standard_deviation()
	             : invGammaDists[i].mode() * 0.05;
    }
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
  Cout << "eigenvalues from symmetric_eigenvalue_decomposition:\n"
       << eigenvalues;
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
    Cout << "Hessian of negative log-likelihood (from misfit):\n" << hessian;
    //Cout << "2x2 determinant = " << hessian(0,0) * hessian(1,1) -
    //  hessian(0,1) * hessian(1,0) << '\n';

    //Cout << "Cholesky factor of prior covariance:\n" << prior_chol_fact;

    Cout << "Prior-preconditioned misfit Hessian:\n" << LT_H_L;
    //Cout << "2x2 determinant = " << LT_H_L(0,0) * LT_H_L(1,1) -
    //  LT_H_L(0,1) * LT_H_L(1,0) << '\n';
    if (num_neglect)
      Cout << "Hessian decomposition neglects " << num_neglect
           << " eigenvalues based on " << eigenval_tol << " tolerance.\n";
  }
  if (output_lev > NORMAL_OUTPUT) {
    Cout << "Positive definite covariance from inverse of Hessian:\n"
	 << covariance;
    //Cout << "2x2 determinant = " << covariance(0,0) * covariance(1,1) -
    //  covariance(0,1) * covariance(1,0) << '\n';
  }

  //return num_neglect;
}


/** Response mapping callback used by RecastModel for MAP pre-solve. Computes 

      -log(post) = -log(like) - log(prior); where
      -log(like) = 1/2*Nr*log(2*pi) + 1/2*log(det(Cov)) + 1/2*r'(Cov^{-1})*r
                 = 1/2*Nr*log(2*pi) + 1/2*log(det(Cov)) + misfit

    (misfit defined as 1/2 r^T (mult^2*Gamma_d)^{-1} r) The passed
    residual_resp has been differenced, interpolated, and covariance-scaled */
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
    if (output_flag)
      Cout << "MAP pre-solve: negative log posterior gradient:\n" << log_grad;
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
    if (output_flag)
      Cout << "MAP pre-solve: negative log posterior Hessian:\n" << log_hess;
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

  RealMatrix filtered_chain;
  if (burnInSamples > 0 || num_skip > 1) {
    filter_chain(acceptanceChain, filtered_chain);
    filter_fnvals(acceptedFnVals, filteredFnVals);
  }
  else {

    filtered_chain = 
      RealMatrix(Teuchos::View, acceptanceChain.values(), 
		 acceptanceChain.stride(), 
		 acceptanceChain.numRows(), acceptanceChain.numCols());

    filteredFnVals = 
      RealMatrix(Teuchos::View, acceptedFnVals.values(), acceptedFnVals.stride(),
		 acceptedFnVals.numRows(), acceptedFnVals.numCols());

  }

  NonDSampling::compute_moments(filtered_chain, chainStats,
				Pecos::STANDARD_MOMENTS);
  NonDSampling::compute_moments(filteredFnVals, fnStats,
				Pecos::STANDARD_MOMENTS);
  if (!requestedProbLevels[0].empty())
    compute_intervals();

  // Print tabular file for the filtered chain
  if (!exportMCMCFilename.empty() || outputLevel >= NORMAL_OUTPUT)
    export_chain(filtered_chain, filteredFnVals);

  if (posteriorStatsKL)
    kl_post_prior(acceptanceChain);
  if (posteriorStatsMutual)
    mutual_info_buildX();
  if (posteriorStatsKDE) 
    calculate_kde();
  if (calModelEvidence)
    calculate_evidence();
}


void NonDBayesCalibration::
filter_chain(const RealMatrix& acceptance_chain, RealMatrix& filtered_chain,
	     int target_length)
{
  // Default behavior: burn in 20% of samples, take only every third point in
  // chain
  int num_mcmc_samples = acceptance_chain.numCols();
  int burn_in_post = int(0.2*num_mcmc_samples);
  int burned_in_post = num_mcmc_samples - burn_in_post;
  int num_skip;
  int mcmc_threshhold = target_length*3;
  if (burned_in_post < mcmc_threshhold) {
    num_skip = 3;
  }
  else {
    // maximally skip to achieve the target length: floor( (count-1)/(len-1) )
    num_skip = (burned_in_post-1)/(target_length-1);
  }
  filter_matrix_cols(acceptance_chain, burn_in_post, num_skip, filtered_chain);
}

void NonDBayesCalibration::filter_chain(const RealMatrix& acceptance_chain,
					RealMatrix& filtered_chain)
{
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  filter_matrix_cols(acceptance_chain, burnin, num_skip, filtered_chain);
}

void NonDBayesCalibration::filter_fnvals(const RealMatrix& accepted_fn_vals,
    					 RealMatrix& filtered_fn_vals)
{
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  filter_matrix_cols(accepted_fn_vals, burnin, num_skip, filtered_fn_vals);
}


void NonDBayesCalibration::
filter_matrix_cols(const RealMatrix& orig_matrix, int start_index,
		   int stride, RealMatrix& filtered_matrix)
{
  // Alternately, could return a view using the stride to skip ahead
  int num_orig = orig_matrix.numCols();
  if (num_orig <= start_index || stride <= 0) {
    Cerr << "\nError: Invalid arguments to NonDBayesCalibraion::"
	 << "filter_matrix_cols()\n";
    abort_handler(METHOD_ERROR);
  }

  // ceil(num_orig/stride), using integer arithmetic since we have ints
  // instead of converting to double (1 + remaining cols divided equally)
  int num_filtered =  1 + (num_orig-start_index-1)/stride;
  filtered_matrix.shape(orig_matrix.numRows(), num_filtered);
  for (int i=start_index, j=0; i<num_orig; i+=stride, ++j) {
      RealVector col_vec =
	Teuchos::getCol(Teuchos::View, const_cast<RealMatrix&>(orig_matrix), i);
      Teuchos::setCol(col_vec, j, filtered_matrix);
  }
}


void NonDBayesCalibration::compute_intervals()
{
  std::ofstream interval_stream("dakota_mcmc_CredPredIntervals.dat");
  std::ostream& screen_stream = Cout;

  // Make accepted function values the rows instead of the columns
  RealMatrix filtered_fn_vals_transpose(filteredFnVals, Teuchos::TRANS);
  // Augment function values with experimental uncertainty for prediction ints
  int num_filtered = filteredFnVals.numCols();
  size_t num_exp = expData.num_experiments();
  size_t num_concatenated = num_exp*num_filtered;

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
  predVals.shapeUninitialized(numFunctions, num_concatenated);
  if (expData.variance_active()) {
    compute_prediction_vals(filteredFnVals, predVals, 
    			    num_filtered, num_exp, num_concatenated);
    RealVector Pred_ave(numFunctions), Pred_stdevs(numFunctions),
	       Pred_interval_minima(numFunctions), 
	       Pred_interval_maxima(numFunctions);
    RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
    compute_col_means(predVals_transpose, Pred_ave);
    compute_col_stdevs(predVals_transpose, Pred_ave, Pred_stdevs);
    interval_stream << "2 sigma Prediction Intervals\n";
    for(size_t i=0; i<numFunctions; ++i){
      Pred_interval_minima[i] = Pred_ave[i] - 2*Pred_stdevs[i];
      Pred_interval_maxima[i] = Pred_ave[i] + 2*Pred_stdevs[i];
      interval_stream << std::setw(width) << resp[i] << " ";
      interval_stream << Pred_interval_minima[i] << ", "
		      << Pred_interval_maxima[i] << '\n';
    }
  }
  interval_stream << "\n";
  // Calculate intervals with sorting - print to screen and interval file
  size_t num_levels = 0;
  for(int i = 0; i < numFunctions; ++i)
    num_levels += requestedProbLevels[i].length();
  if (num_levels > 0)
    print_intervals_file(interval_stream, filtered_fn_vals_transpose, 
      			   predVals, num_filtered, num_concatenated);

  interval_stream << "acceptedVals = " << acceptedFnVals << '\n';
  interval_stream << "predVals = " << predVals << '\n';
}

void NonDBayesCalibration::compute_prediction_vals
(RealMatrix& filtered_fn_vals, RealMatrix& predVals, 
int num_filtered, size_t num_exp, size_t num_concatenated)
{
  // Read std_dev and correl matrices if specified for experiments
  RealVectorArray std_deviations;
  RealSymMatrixArray correl_matrices;
  //if (calibrationData && expData.variance_active()){
  expData.cov_std_deviation(std_deviations);
  expData.cov_as_correlation(correl_matrices);
  //}
  
  // Augment function values with experimental uncertainty for prediction ints
  // Generate normal errors using LHS
  /*int num_res = residualModel.response_size();
    RealVector means_vec(num_res), lower_bnds(num_res), upper_bnds(num_res);
    */
  RealVector means_vec(numFunctions), lower_bnds(numFunctions), 
	     upper_bnds(numFunctions);
  means_vec.putScalar(0.0);
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  lower_bnds.putScalar(-dbl_inf);
  upper_bnds.putScalar( dbl_inf);
  RealMatrix lhs_normal_samples;
  unsigned short sample_type = SUBMETHOD_LHS;
  short sample_ranks_mode = 0; //IGNORE RANKS
  Pecos::LHSDriver lhsDriver; // the C++ wrapper for the F90 LHS library
  size_t e, s, r, cntr = 0;
  lhsDriver.seed(randomSeed);
  lhsDriver.initialize("lhs", sample_ranks_mode, true);
  for (e=0; e<num_exp; ++e) {
    //int lhs_seed = (randomSeed > 0) ? randomSeed : generate_system_seed();
    lhsDriver.generate_normal_samples(means_vec, std_deviations[e], lower_bnds,
              upper_bnds, correl_matrices[e], num_filtered, lhs_normal_samples);
    for (s=0; s<num_filtered; ++s, ++cntr)
      for (r=0; r<numFunctions; ++r)
	predVals(r,cntr) = filtered_fn_vals(r,s) + lhs_normal_samples(r,s);
  }
}

/** Print tabular file with filtered chain, function values, and pred values */
void NonDBayesCalibration::
export_chain(RealMatrix& filtered_chain, RealMatrix& filtered_fn_vals)
{
  String mcmc_filename = 
    exportMCMCFilename.empty() ? "dakota_mcmc_tabular.dat" : exportMCMCFilename;
  std::ofstream export_mcmc_stream;
  TabularIO::open_file(export_mcmc_stream, mcmc_filename,
		       "NonDBayesCalibration chain export");

  // Use a Variables object for proper tabular formatting.
  // The residual model includes hyper-parameters, if present
  Variables output_vars = residualModel.current_variables().copy();

  // When outputting only chain responses
  const StringArray& resp_labels = 
    mcmcModel.current_response().function_labels();
  // When outputting experimental responses
  /*
  size_t num_exp = expData.num_experiments();
  StringArray resp_labels;
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  for (size_t i=0; i<num_exp+1; ++i){
    for (size_t k=0; k<numFunctions; ++k){
      resp_labels.push_back(resp[k]);
    }
  }
  */

  TabularIO::write_header_tabular(export_mcmc_stream, output_vars, resp_labels,
				  "mcmc_id", "interface", exportMCMCFormat);

  size_t wpp4 = write_precision+4;
  export_mcmc_stream << std::setprecision(write_precision) 
		     << std::resetiosflags(std::ios::floatfield);

  int num_filtered = filtered_chain.numCols();
  for (int i=0; i<num_filtered; ++i) {
    TabularIO::write_leading_columns(export_mcmc_stream, i+1,
				     mcmcModel.interface_id(),
				     exportMCMCFormat);
    RealVector accept_pt = Teuchos::getCol(Teuchos::View, filtered_chain, i);
    output_vars.continuous_variables(accept_pt);
    output_vars.write_tabular(export_mcmc_stream);
    // Write function values to filtered_tabular
    RealVector col_vec = Teuchos::getCol(Teuchos::View, filtered_fn_vals, i);
    for (size_t j=0; j<numFunctions; ++j){
      export_mcmc_stream << std::setw(wpp4) << col_vec[j] << ' ';
    }      
    // Write predicted values to filtered_tabular
    // When outputting experimental responses 
    /*
    for (size_t j =0; j<num_exp; ++j){
      for (size_t k = 0; k<numFunctions; ++k){
	int col_index = j*num_filtered+i;
        const RealVector& col_vec = Teuchos::getCol(Teuchos::View, 
      				    predVals, col_index);
  	export_mcmc_stream << std::setw(wpp4) << col_vec[k] << ' ';
      }
    }
    */
    export_mcmc_stream << '\n';
  }

  TabularIO::close_file(export_mcmc_stream, mcmc_filename,
			"NonDQUESOBayesCalibration chain export");
}

void NonDBayesCalibration::
calculate_kde()
{
  RealVector pdf_results;
  Pecos::GaussianKDE kde;
  //Cout << "Accepted Chain in KDE " << acceptanceChain <<  '\n';
  //Cout << "Accepted Fn Values in KDE " << acceptedFnVals <<  '\n';
  std::ofstream export_kde;
  size_t wpp4 = write_precision+4;
  StringArray var_labels;
        copy_data(residualModel.continuous_variable_labels(),var_labels);
  const StringArray& resp_labels = 
    		     mcmcModel.current_response().function_labels();
  TabularIO::open_file(export_kde, "kde_posterior.dat",
			"NonDBayesCalibration kde posterior export");
  
  int num_rows = acceptanceChain.numCols();
  int num_vars = acceptanceChain.numRows();
  RealMatrix current_var;
  current_var.shapeUninitialized(1,num_rows);
  for (int i=0; i<num_vars; ++i){
    for (int j=0; j<num_rows; j++) 
      current_var(0,j)=acceptanceChain(i,j);
    //Cout << current_var;
    kde.initialize(current_var,Teuchos::TRANS);
    kde.pdf(current_var, pdf_results,Teuchos::TRANS);
    //Cout << pdf_results;
    export_kde << var_labels[i] << "  KDE PDF estimate  " << '\n';
    //TabularIO::
    //  write_header_tabular(export_kde, output_vars(i), "KDE PDF estimate");
    for (int j=0; j<num_rows; j++) 
      export_kde <<  current_var(0,j) << "    " << pdf_results(j) << '\n';
    export_kde << '\n';
  }
  int num_responses = acceptedFnVals.numRows();
  RealMatrix current_resp;
  current_resp.shapeUninitialized(1,num_rows);
  for (int i=0; i<num_responses; ++i){
    for (int j=0; j<num_rows; j++) 
      current_resp(0,j)=acceptedFnVals(i,j);
    //Cout << current_resp;
    //RealMatrix& col_resp = Teuchos::getCol(Teuchos::View, acceptedFnVals, i);
    //kde.initialize(current_resp, Teuchos::TRANS);
    kde.initialize(current_resp,Teuchos::TRANS);
    kde.pdf(current_resp, pdf_results,Teuchos::TRANS);
    //Cout << pdf_results;
    export_kde << resp_labels[i] << "  KDE PDF estimate  " << '\n';
    //TabularIO::
    //  write_header_tabular(export_kde, resp_labels(i), "KDE PDF estimate");
    for (int j=0; j<num_rows; j++) 
      export_kde <<  current_resp(0,j) << "    " << pdf_results(j) << '\n';
    export_kde << '\n';
  }
  TabularIO::close_file(export_kde, "kde_posterior.dat",
			"NonDBayesCalibration kde posterior export");
  
}

void NonDBayesCalibration::calculate_evidence()
{
  // set default to MC approximation if neither method specified
  if (!calModelEvidMC && !calModelEvidLaplace)
    calModelEvidMC = true;
  if (calModelEvidMC) {
    //int num_prior_samples = chainSamples; //KAM: replace later
    int num_prior_samples = (evidenceSamples>0) ? evidenceSamples : chainSamples;
    int num_params = numContinuousVars + numHyperparams;
    // Draw samples from prior distribution 
    RealMatrix prior_dist_samples(num_params, num_prior_samples);
    prior_sample_matrix(prior_dist_samples);
    // Calculate likelihood for each sample
    double sum_like = 0.;
    for (int i = 0; i < num_prior_samples; i++) {
      RealVector params = Teuchos::getCol(Teuchos::View, prior_dist_samples, i);
      RealVector cont_params = params;
      cont_params.resize(numContinuousVars);  
      residualModel.continuous_variables(cont_params);
      residualModel.evaluate();
      RealVector residual = residualModel.current_response().function_values();
      double log_like = log_likelihood(residual, params);
      sum_like += std::exp(log_like);
    }
    double evidence = sum_like/num_prior_samples;
    Cout << "Model evidence (Monte Carlo) = " << evidence << '\n';
    //Cout << "num samples = " << num_prior_samples << '\n';
  }
  if (calModelEvidLaplace) {
    if (obsErrorMultiplierMode > CALIBRATE_NONE) {
      Cout << "The Laplace approximation of model evidence currently " 
           << "does not work when error multipliers are specified."<< std::endl;
      abort_handler(METHOD_ERROR);
    } 
    //if (mapOptAlgOverride == SUBMETHOD_NONE) {
    //  Cout << "You must specify a pre-solve method for the Laplace " 
    //       << "approximation of model evidence." << std::endl;
    //  abort_handler(METHOD_ERROR);
    //} 
    Cout << "Starting Laplace approximation of model evidence, first " 
         << "\nobtain MAP point from pre-solve.\n";
    const RealVector& map_c_vars
      = mapOptimizer.variables_results().continuous_variables();
    //estimate likelihood at MAP point: 
    residualModel.continuous_variables(map_c_vars);
    ActiveSet resAS = residualModel.current_response().active_set();
    resAS.request_values(7);
    residualModel.evaluate(resAS);
    RealVector residual = residualModel.current_response().function_values();
    Real laplace_like = log_likelihood(residual, map_c_vars);
    //obtain prior density at MAP point: 
    Real laplace_prior =  nonDBayesInstance->log_prior_density(map_c_vars);
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Residual at MAP point" << residualModel.current_response() << '\n';
      Cout << "Log_likelihood at MAP Point" << laplace_like << '\n';
      Cout << "Laplace_prior " << laplace_prior << "\n";
    }
    Response nlpost_resp = negLogPostModel.current_response().copy();
    ActiveSet as2 = nlpost_resp.active_set();
    as2.request_values(7);
    nlpost_resp.active_set(as2);
    neg_log_post_resp_mapping(mapOptimizer.variables_results(), mapOptimizer.variables_results(), 
      residualModel.current_response(), nlpost_resp);
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Negative log posterior function values " << nlpost_resp.function_values() << '\n';
      Cout << "Negative log posterior Hessian " << nlpost_resp.function_hessian_view(0) << '\n';
      //Cout << nlpost_resp << '\n';
    }
    RealSymMatrix log_hess;
    nonDBayesInstance->
      expData.build_hessian_of_sum_square_residuals(residualModel.current_response(), log_hess);
    // Add the contribution from 1/2*log(det(Cov))
    nonDBayesInstance->expData.half_log_cov_det_hessian
      (0, nonDBayesInstance->obsErrorMultiplierMode, 
      nonDBayesInstance->numContinuousVars, log_hess);
    // Add the contribution from -log(prior)
    nonDBayesInstance->augment_hessian_with_log_prior(log_hess, map_c_vars);
    Cout << "Laplace approximation: negative log posterior Hessian:\n"
         <<  log_hess << "\n";
    CovarianceMatrix local_log_post_hess; 
    // for now, the matrix passed to set_covariance is a RealMatrix, not RealSymMatrix
    RealMatrix clog_hess(numContinuousVars, numContinuousVars);
    for (int i=0; i<numContinuousVars; i++) {
      for (int j=0; j<numContinuousVars; j++) {
        clog_hess(i,j)=log_hess(i,j);
      }
    }
    try {
      local_log_post_hess.set_covariance(const_cast<RealMatrix&>(clog_hess)); 
      Cout << "log determinant post" << local_log_post_hess.log_determinant() <<std::endl; 
      double lpres= laplace_prior+laplace_like+numContinuousVars*HALF_LOG_2PI-0.5*local_log_post_hess.log_determinant();
      Cout << "Model evidence (Laplace) = " << std::exp(lpres) <<  '\n';
    }
    catch (const std::runtime_error& re){
      Cout << "Can't compute model evidence because the Hessian of the negative log posterior distribution " 
           << "is not positive definite. " << re.what();
    }
  }
}

void NonDBayesCalibration::print_intervals_file
(std::ostream& s, RealMatrix& filteredFnVals_transpose, 
 RealMatrix& predVals, int num_filtered, size_t num_concatenated)
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
  if (expData.variance_active()) {
    RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
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
  if (expData.variance_active() ) {
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
        s << std::setw(width) <<' '<< " Response Level    Probability Level\n";
        s << std::setw(width) <<' '<< " ----------------- -----------------\n";
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
}

void NonDBayesCalibration::print_results(std::ostream& s, short results_state)
{
  // Print chain moments
  StringArray combined_labels;
  copy_data(residualModel.continuous_variable_labels(), combined_labels);
  NonDSampling::print_moments(s, chainStats, RealMatrix(), 
      "posterior variable", Pecos::STANDARD_MOMENTS, combined_labels, false); 
  // Print response moments
  StringArray resp_labels = mcmcModel.current_response().function_labels();
  NonDSampling::print_moments(s, fnStats, RealMatrix(), 
      "response function", Pecos::STANDARD_MOMENTS, resp_labels, false); 
  
  // Print chain diagnostics for variables
  if (chainDiagnostics)
    print_chain_diagnostics(s);
  // Print credibility and prediction intervals to screen
  if (requestedProbLevels[0].length() > 0 && outputLevel >= NORMAL_OUTPUT) {
    int num_filtered = filteredFnVals.numCols();
    RealMatrix filteredFnVals_transpose(filteredFnVals, Teuchos::TRANS);
    RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
    print_intervals_screen(s, filteredFnVals_transpose,
			   predVals_transpose, num_filtered);
  }

  // Print posterior stats
  if (posteriorStatsKL)
    print_kl(s);
}

void NonDBayesCalibration::
print_variables(std::ostream& s, const RealVector& c_vars)
{
  StringMultiArrayConstView cv_labels =
    iteratedModel.continuous_variable_labels();
  // the residualModel includes any hyper-parameters
  StringArray combined_labels;
  copy_data(residualModel.continuous_variable_labels(), combined_labels);

  size_t wpp7 = write_precision+7;

  // print MAP for continuous random variables
  if (standardizedSpace) {
    RealVector u_rv(Teuchos::View, c_vars.values(), numContinuousVars);
    RealVector x_rv;
    mcmcModel.trans_U_to_X(u_rv, x_rv);
    write_data(Cout, x_rv, cv_labels);
  }
  else
    for (size_t j=0; j<numContinuousVars; ++j)
      s << "                     " << std::setw(wpp7) << c_vars[j]
	<< ' ' << cv_labels[j] << '\n';
  // print MAP for hyper-parameters (e.g., observation error params)
  for (size_t j=0; j<numHyperparams; ++j)
    s << "                     " << std::setw(wpp7)
      << c_vars[numContinuousVars+j] << ' '
      << combined_labels[numContinuousVars + j] << '\n';
}

void NonDBayesCalibration::kl_post_prior(RealMatrix& acceptanceChain)
{
  // sub-sample posterior chain
  int num_params = numContinuousVars + numHyperparams;
  int num_post_samples = acceptanceChain.numCols();
  int burn_in_post = int(0.2*num_post_samples);
  int burned_in_post = num_post_samples - burn_in_post;
  RealMatrix knn_post_samples;
  RealMatrix prior_dist_samples;
  if (num_post_samples < 18750){ // Aiming for num_filtered = 5000 
    int num_skip = 3;
    int num_filtered = int(burned_in_post/num_skip);
    int num_prior_samples = num_filtered*125;
    knn_post_samples.shape(num_params, num_filtered);
    prior_dist_samples.shape(num_params, num_prior_samples);
    int j = 0;
    int it_cntr = 0;
    for (int i = burn_in_post+1; i < num_post_samples; ++i){
      ++it_cntr;
      if (it_cntr % num_skip == 0){
	RealVector param_vec = Teuchos::getCol(Teuchos::View,
	  			        acceptanceChain, i);
	Teuchos::setCol(param_vec, j, knn_post_samples);
	++j;
      }
    }
  }
  else{
    int num_skip = int(burned_in_post/5000);
    int num_filtered = int(burned_in_post/num_skip);
    int num_prior_samples = num_filtered*125;
    knn_post_samples.shapeUninitialized(num_params, num_filtered);
    prior_dist_samples.shapeUninitialized(num_params, num_prior_samples);
    int j = 0;
    int it_cntr = 0;
    for (int i = burn_in_post; i < num_post_samples; ++i){
      if (it_cntr % num_skip == 0){
	  ++it_cntr;
	  RealVector param_vec = Teuchos::getCol(Teuchos::View,
	    			        acceptanceChain, i);
	  Teuchos::setCol(param_vec, j, knn_post_samples);
	  j++;
      }
    }
  }
  
  // produce matrix of prior samples
  prior_sample_matrix(prior_dist_samples);
  // compute knn kl-div between prior and posterior
  kl_est = knn_kl_div(knn_post_samples, prior_dist_samples, numContinuousVars);
}

void NonDBayesCalibration::prior_sample_matrix(RealMatrix& prior_dist_samples)
{
  // Create matrix containing samples from the prior distribution
  boost::mt19937 rnumGenerator;
  int num_params = prior_dist_samples.numRows(); 
  int num_samples = prior_dist_samples.numCols();
  RealVector vec(num_params);
  rnumGenerator.seed(randomSeed);
  for(int i = 0; i < num_samples; ++i){
    prior_sample(rnumGenerator, vec);
    Teuchos::setCol(vec, i, prior_dist_samples);
  }
}

Real NonDBayesCalibration::knn_kl_div(RealMatrix& distX_samples,
    			 	RealMatrix& distY_samples, size_t dim)
{
  approxnn::normSelector::instance().method(approxnn::L2_NORM);

  size_t NX = distX_samples.numCols();
  size_t NY = distY_samples.numCols();
  //size_t dim = numContinuousVars; 
  
  // k is recorded for each distance so that if it needs to be adapted
  // (if kNN dist = 0), we can calculate the correction term 
  IntVector k_vec_XY(NX);
  k_vec_XY.putScalar(6); //k default set to 6
  IntVector k_vec_XX(NX);
  k_vec_XX.putScalar(7); //k default set to 6
  			 //1st neighbor is self, so need k+1 for XtoX
  double eps = 0.0; //default ann error
   
  // Copy distX samples into ANN data types
  ANNpointArray dataX;
  dataX = annAllocPts(NX, dim);
  RealVector col(dim);
  for (int i = 0; i < NX; i++){
    col = Teuchos::getCol(Teuchos::View, distX_samples, i);
    for (int j = 0; j < dim; j++){
      dataX[i][j] = col[j];
    }
  } 
  // Copy distY samples into ANN data types
  ANNpointArray dataY;
  dataY = annAllocPts(NY, dim);
  for (int i = 0; i < NY; i++){
    col = Teuchos::getCol(Teuchos::View, distY_samples, i);
    for (int j = 0; j < dim; j++){
      dataY[i][j] = col[j];
    }
  } 
  
  // calculate vector of kNN distances from dist1 to dist2
  RealVector XtoYdistances(NX);
  ann_dist(dataX, dataY, XtoYdistances, NX, NY, dim, k_vec_XY, eps);
  // calculate vector of kNN distances from dist1 to itself
  RealVector XtoXdistances(NX);
  ann_dist(dataX, dataX, XtoXdistances, NX, NX, dim, k_vec_XX, eps);
  
  double log_sum = 0;
  double digamma_sum = 0;
  double rat;
  for (int i = 0; i < NX; i++){
    rat = XtoYdistances[i]/XtoXdistances[i];
    log_sum += log(XtoYdistances[i]/XtoXdistances[i]);
    if (k_vec_XY[i] != (k_vec_XX[i]-1)){ //XtoX: first NN is self
      double psiX = boost::math::digamma(k_vec_XX[i]-1);
      double psiY = boost::math::digamma(k_vec_XY[i]);
      digamma_sum += psiX - psiY;
    }
  }
  Real Dkl_est = 0.0;
  Dkl_est = (double(dim)*log_sum + digamma_sum)/double(NX)
          + log( double(NY)/(double(NX)-1) );

  annDeallocPts( dataX );
  annDeallocPts( dataY );

  approxnn::normSelector::instance().reset();

  return Dkl_est;
}

void NonDBayesCalibration::mutual_info_buildX()
{
  /* Build matrix X, containing samples of the two distributions being
   * considered in the mutual info calculation. Each column has the form
   * X_i = [x1_i x2_i ... xn_i y1_i y2_i ... ym_i]
   */

  int num_params = numContinuousVars + numHyperparams;
  int num_samples = 1000;
  boost::mt19937 rnumGenerator;
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(2*num_params, num_samples);
  RealVector vec(num_params);
  RealVector col_vec(2*num_params);
  rnumGenerator.seed(randomSeed);
  for (int i = 0; i < num_samples; ++i){
    prior_sample(rnumGenerator, vec);
    for (int j = 0; j < num_params; ++j){
      col_vec[j] = vec[j];
    }
    prior_sample(rnumGenerator, vec);
    for (int j = 0; j < num_params; ++j){
      col_vec[j+1] = vec[j];
    }
    Teuchos::setCol(col_vec, i, Xmatrix);
  }

  // Test matrix
  /*
  int num_samples = acceptanceChain.numCols();
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(2*num_params, num_samples);
  RealVector vec(num_params);
  RealVector col_vec(2*num_params);
  for (int i = 0; i < num_samples-1; ++i){
    vec = Teuchos::getCol(Teuchos::View, acceptanceChain, i);
    for (int j = 0; j < num_params; ++j){
      col_vec[j] = vec[j]; //offset values by 1
    }
    vec = Teuchos::getCol(Teuchos::View, acceptanceChain, i+1);
    for (int j = 0; j < num_params; ++j){
      col_vec[j+num_params] = vec[j];
    }
    Teuchos::setCol(col_vec, i, Xmatrix);
  }
  // last column
  vec = Teuchos::getCol(Teuchos::View, acceptanceChain, num_samples-1);
  for (int j = 0; j < num_params; ++j){
    col_vec[j] = vec[j]; //offset values by 1
  }
  vec = Teuchos::getCol(Teuchos::View, acceptanceChain, 0);
  for (int j = 0; j < num_params; ++j){
    col_vec[j+num_params] = vec[j];
  }
  Teuchos::setCol(col_vec, num_samples-1, Xmatrix);
  */

  //test_stream << "Xmatrix = " << Xmatrix << '\n';

  Real mutualinfo_est =
    knn_mutual_info(Xmatrix, num_params, num_params, mutualInfoAlg);
  Cout << "MI est = " << mutualinfo_est << '\n';

}

Real NonDBayesCalibration::knn_mutual_info(RealMatrix& Xmatrix, int dimX,
    int dimY, unsigned short alg)
{
  approxnn::normSelector::instance().method(approxnn::LINF_NORM);

  //std::ofstream test_stream("kam1.txt");
  //test_stream << "Xmatrix = " << Xmatrix << '\n';
  //Cout << "Xmatrix = " << Xmatrix << '\n';

  int num_samples = Xmatrix.numCols();
  int dim = dimX + dimY;

  // Cast Xmatrix into ANN data structure
  ANNpointArray dataXY;
  dataXY = annAllocPts(num_samples, dim);
  RealVector col(dim);
  for (int i = 0; i < num_samples; i++){
    col = Teuchos::getCol(Teuchos::View, Xmatrix, i);
    for (int j = 0; j < dim; j++){
      dataXY[i][j] = col[j];
    }
  }

  // Normalize data
  ANNpoint meanXY, stdXY;
  meanXY = annAllocPt(dim); //means
  for (int i = 0; i < num_samples; i++){
    for(int j = 0; j < dim; j++){
      meanXY[j] += dataXY[i][j];
    }
  }
  for (int j = 0; j < dim; j++){
    meanXY[j] = meanXY[j]/double(num_samples);
    //Cout << "mean" << j << " = " << meanXY[j] << '\n';
  }
  stdXY = annAllocPt(dim); //standard deviations
  for (int i = 0; i < num_samples; i++){
    for (int j = 0; j < dim; j++){
      stdXY[j] += pow (dataXY[i][j] - meanXY[j], 2.0);
    }
  }
  for (int j = 0; j < dim; j++){
    stdXY[j] = sqrt( stdXY[j]/(double(num_samples)-1.0) );
    //Cout << "std" << j << " = " << stdXY[j] << '\n';
  }
  for (int i = 0; i < num_samples; i++){
    for (int j = 0; j < dim; j++){
      dataXY[i][j] = ( dataXY[i][j] - meanXY[j] )/stdXY[j];
    }
    //Cout << "dataXY = " << dataXY[i][0] << '\n';
  }

  // Get knn-distances for Xmatrix
  RealVector XYdistances(num_samples);
  Int2DArray XYindices(num_samples);
  IntVector k_vec(num_samples);
  int k = 6;
  k_vec.putScalar(k); // for self distances, need k+1
  double eps = 0.0;
  ann_dist(dataXY, dataXY, XYdistances, XYindices, num_samples, num_samples, 
           dim, k_vec, eps);

  // Build marginals
  ANNpointArray dataX, dataY;
  dataX = annAllocPts(num_samples, dimX);
  dataY = annAllocPts(num_samples, dimY);
  //RealMatrix chainX(dimX, num_samples);
  //RealMatrix chainY(dimY, num_samples);
  for(int i = 0; i < num_samples; i++){
    col = Teuchos::getCol(Teuchos::View, Xmatrix, i);
    //Cout << "col = " << col << '\n';
    for(int j = 0; j < dimX; j++){
      //dataX[i][j] = dataXY[i][j];
      //Cout << "col " << j << " = " << col[j] << '\n';
      //chainX[i][j] = col[j];
      dataX[i][j] = col[j];
    }
    for(int j = 0; j < dimY; j++){
      //dataY[i][j] = dataXY[i][dimX+j];
      //chainY[i][j] = col[dimX+j];
      dataY[i][j] = col[dimX+j];
    }
  }
  //Cout << "chainX = " << chainX;
  ANNkd_tree* kdTreeX;
  ANNkd_tree* kdTreeY;
  kdTreeX = new ANNkd_tree(dataX, num_samples, dimX);
  kdTreeY = new ANNkd_tree(dataY, num_samples, dimY);

  double marg_sum = 0.0;
  int n_x, n_y;
  for(int i = 0; i < num_samples; i++){
    if (alg == MI_ALG_KSG2) { //alg=1, ksg2
      IntArray XYind_i = XYindices[i];	
      RealArray ex_vec(XYind_i.size()-1);
      RealArray ey_vec(XYind_i.size()-1);
      for(int j = 1; j < XYind_i.size(); j ++) {
	ex_vec[j-1] = annDist(dimX, dataX[i], dataX[XYind_i[j]]);
	ey_vec[j-1] = annDist(dimY, dataY[i], dataY[XYind_i[j]]);
      }
      ANNdist e_x = *std::max_element(ex_vec.begin(), ex_vec.end());
      ANNdist e_y = *std::max_element(ey_vec.begin(), ey_vec.end());
      /*
      ANNdist e = max(e_x, e_y);
      n_x = kdTreeX->annkFRSearch(dataX[i], e, 0, NULL, NULL, eps);
      n_y = kdTreeY->annkFRSearch(dataY[i], e, 0, NULL, NULL, eps);
      */
      n_x = kdTreeX->annkFRSearch(dataX[i], e_x, 0, NULL, NULL, eps);
      n_y = kdTreeY->annkFRSearch(dataY[i], e_y, 0, NULL, NULL, eps);
    }
    else { //alg=0, ksg1
      n_x = kdTreeX->annkFRSearch(dataX[i], XYdistances[i], 0, NULL, 
  				    NULL, eps);
      n_y = kdTreeY->annkFRSearch(dataY[i], XYdistances[i], 0, NULL,
  				    NULL, eps);
    }
    double psiX = boost::math::digamma(n_x);
    double psiY = boost::math::digamma(n_y);
    //double psiX = boost::math::digamma(n_x+1);
    //double psiY = boost::math::digamma(n_y+1);
    marg_sum += psiX + psiY;
    //test_stream <<"i = "<< i <<", nx = "<< n_x <<", ny = "<< n_y <<'\n';
    //test_stream << "psiX = " << psiX << '\n';
    //test_stream << "psiY = " << psiY << '\n';
  }
  double psik = boost::math::digamma(k);
  double psiN = boost::math::digamma(num_samples);
  double MI_est = psik - (marg_sum/double(num_samples)) + psiN;
  if (alg == MI_ALG_KSG2) {
    MI_est = MI_est - 1/double(k);
  }
  //test_stream << "psi_k = " << psik << '\n';
  //test_stream << "marg_sum = " << marg_sum << '\n';
  //test_stream << "psiN = " << psiN << '\n';
  //test_stream << "MI_est = " << MI_est << '\n';

  // Dealloc memory
  delete kdTreeX;
  delete kdTreeY;
  annDeallocPts(dataX);
  annDeallocPts(dataY);
  annDeallocPts(dataXY);

  approxnn::normSelector::instance().reset();

  // Compare to dkl
  /*
  double kl_est = knn_kl_div(Xmatrix, Xmatrix);
  test_stream << "KL = " << kl_est << '\n';
  Cout << "KL = " << kl_est << '\n';
  */
  return MI_est;

}

void NonDBayesCalibration::ann_dist(const ANNpointArray matrix1, 
     const ANNpointArray matrix2, RealVector& distances, int NX, int NY, 
     int dim2, IntVector& k_vec, double eps)
{

  ANNkd_tree* kdTree;
  kdTree = new ANNkd_tree( matrix2, NY, dim2 );
  for (unsigned int i = 0; i < NX; ++i){
    int k_i = k_vec[i] ;
    ANNdistArray knn_dist = new ANNdist[k_i+1];
    ANNidxArray knn_ind = new ANNidx[k_i+1];
    //calc min number of distances needed
    kdTree->annkSearch(matrix1[ i ], k_i+1, knn_ind, knn_dist, eps);
    double dist = knn_dist[k_i];
    if (dist == 0.0){
      ANNdistArray knn_dist_i = new ANNdist[NY];
      ANNidxArray knn_ind_i = new ANNidx[NY];
      //calc distances for whole array
      kdTree->annkSearch(matrix1[ i ], NY, knn_ind_i, knn_dist_i, eps); 
      for (unsigned int j = k_i+1; j < NY; ++j){
	if (knn_dist_i[j] > 0.0){
	  dist = knn_dist_i[j];
	  k_vec[i] = j;
	  break;
	}
      }
      delete [] knn_ind_i;
      delete [] knn_dist_i;
    }
    distances[i] = dist;
    delete [] knn_ind;
    delete [] knn_dist;
  }
  delete kdTree;
  annClose();
}

void NonDBayesCalibration::ann_dist(const ANNpointArray matrix1, 
     const ANNpointArray matrix2, RealVector& distances, Int2DArray& indices, 
     int NX, int NY, int dim2, IntVector& k_vec, 
     double eps)
{
  ANNkd_tree* kdTree;
  kdTree = new ANNkd_tree( matrix2, NY, dim2 );
  for (unsigned int i = 0; i < NX; ++i){
    int k_i = k_vec[i] ;
    ANNdistArray knn_dist = new ANNdist[k_i+1];
    ANNidxArray knn_ind = new ANNidx[k_i+1];
    //calc min number of distances needed
    kdTree->annkSearch(matrix1[ i ], k_i+1, knn_ind, knn_dist, eps);
    double dist = knn_dist[k_i];
    IntArray ind(k_i+1);
    for (int j = 0; j < k_i+1; j ++) 
      ind[j] = knn_ind[j];
    if (dist == 0.0){
      ANNdistArray knn_dist_i = new ANNdist[NY];
      ANNidxArray knn_ind_i = new ANNidx[NY];
      //calc distances for whole array
      kdTree->annkSearch(matrix1[ i ], NY, knn_ind_i, knn_dist_i, eps); 
      for (unsigned int j = k_i+1; j < NY; ++j){
	if (knn_dist_i[j] > 0.0){
	  dist = knn_dist_i[j];
	  ind.resize(j);
	  for (int k = 0; k < j; k ++)  
	    ind[k] = knn_ind_i[k];
	  k_vec[i] = j;
	  break;
	}
      }
      delete [] knn_ind_i;
      delete [] knn_dist_i;
    }
    distances[i] = dist;
    indices[i] = ind;
    delete [] knn_ind;
    delete [] knn_dist;
  }
  delete kdTree;
  annClose();
}

void NonDBayesCalibration::print_kl(std::ostream& s)
{
  s << "Information gained from prior to posterior = " << kl_est;
  s << '\n';
}

void NonDBayesCalibration::print_chain_diagnostics(std::ostream& s)
{
  s << "\nChain diagnostics\n";
  if (chainDiagnosticsCI)
    print_batch_means_intervals(s);
}

void NonDBayesCalibration::print_batch_means_intervals(std::ostream& s)
{
  size_t width = write_precision+7;
  Real alpha = 0.95;
  
  int num_vars = acceptanceChain.numRows();
  StringArray var_labels;
  copy_data(residualModel.continuous_variable_labels(),	var_labels);
  RealMatrix variables_mean_interval_mat, variables_mean_batch_means;
  batch_means_interval(acceptanceChain, variables_mean_interval_mat,
                       variables_mean_batch_means, 1, alpha);
  RealMatrix variables_var_interval_mat, variables_var_batch_means;
  batch_means_interval(acceptanceChain, variables_var_interval_mat,
                       variables_var_batch_means, 2, alpha);
  
  int num_responses = acceptedFnVals.numRows();
  StringArray resp_labels = mcmcModel.current_response().function_labels();
  RealMatrix responses_mean_interval_mat, responses_mean_batch_means;
  batch_means_interval(acceptedFnVals, responses_mean_interval_mat,
                       responses_mean_batch_means, 1, alpha);
  RealMatrix responses_var_interval_mat, responses_var_batch_means;
  batch_means_interval(acceptedFnVals, responses_var_interval_mat,
                       responses_var_batch_means, 2, alpha);

  if (outputLevel >= DEBUG_OUTPUT) {
    for (int i = 0; i < num_vars; i++) {
      s << "\tBatch means of mean for variable ";
      s << var_labels[i] << '\n';
      const RealVector& mean_vec = Teuchos::getCol(Teuchos::View,
                                  variables_mean_batch_means, i);
      s << mean_vec ;
      s << "\tBatch means of variance for variable ";
      const RealVector& var_vec = Teuchos::getCol(Teuchos::View,
                                  variables_var_batch_means, i);
      s << var_labels[i] << '\n';
      s << var_vec ;
    }
    for (int i = 0; i < num_responses; i++) {
      s << "\tBatch means of mean for response ";
      s << resp_labels[i] << '\n';
      const RealVector mean_vec = Teuchos::getCol(Teuchos::View,
                                  responses_mean_batch_means, i);
      s << mean_vec ;
      s << "\tBatch means of variance for response ";
      s << resp_labels[i] << '\n';
      const RealVector var_vec = Teuchos::getCol(Teuchos::View, 
                                 responses_var_batch_means, i);
      s << var_vec ;
    }
  }

  s << "\t95% Confidence Intervals of means\n";
  for (int i = 0; i < num_vars; i++) {
    RealVector col_vec = Teuchos::getCol(Teuchos::View,
                                         variables_mean_interval_mat, i);
    s << '\t' << std::setw(width) << var_labels[i]
      << " = [" << col_vec[0] << ", " << col_vec[1] << "]\n";
  }
  for (int i = 0; i < num_responses; i++) {
    RealVector col_vec = Teuchos::getCol(Teuchos::View,
                                         responses_mean_interval_mat, i);
    s << '\t' << std::setw(width) << resp_labels[i]
      << " = [" << col_vec[0] << ", " << col_vec[1] << "]\n";
  }
  s << "\t95% Confidence Intervals of variances\n";
  for (int i = 0; i < num_vars; i++) {
    RealVector col_vec = Teuchos::getCol(Teuchos::View,
                                         variables_var_interval_mat, i);
    s << '\t' << std::setw(width) << var_labels[i]
      << " = [" << col_vec[0] << ", " << col_vec[1] << "]\n";
  }
  for (int i = 0; i < num_responses; i++) {
    RealVector col_vec = Teuchos::getCol(Teuchos::View,
                                         responses_var_interval_mat, i);
    s << '\t' << std::setw(width) << resp_labels[i]
      << " = [" << col_vec[0] << ", " << col_vec[1] << "]\n";
  }
}


/** Wrap the residualModel in a scaling transformation, such that
    residualModel now contains a scaling recast model. */
void NonDBayesCalibration::scale_model()
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing scaling transformation" << std::endl;

  // residualModel becomes the sub-model of a RecastModel:
  residualModel.assign_rep(std::make_shared<ScalingModel>(residualModel));
  // scalingModel = residualModel;
}


/** Setup Recast for weighting model.  The weighting transformation
    doesn't resize, and makes no vars, active set or secondary mapping.
    All indices are one-to-one mapped (no change in counts). */
void NonDBayesCalibration::weight_model()
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing weighting transformation" << std::endl;

  // we assume sqrt(w_i) will be applied to each residual, therefore:
  const RealVector& lsq_weights = residualModel.primary_response_fn_weights();
  for (int i=0; i<lsq_weights.length(); ++i)
    if (lsq_weights[i] < 0) {
      Cerr << "\nError: Calibration term weights must be nonnegative. "
	   << "Specified weights are:\n" << lsq_weights << '\n';
      abort_handler(-1);
    }

  // TODO: pass sqrt to WeightingModel
  residualModel.assign_rep(std::make_shared<WeightingModel>(residualModel));
}

} // namespace Dakota
