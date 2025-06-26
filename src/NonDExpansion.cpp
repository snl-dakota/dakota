/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDExpansion.hpp"

#include "DakotaResponse.hpp"
#include "DataFitSurrModel.hpp"
#include "DiscrepancyCorrection.hpp"
#include "NatafTransformation.hpp"
#include "NonDAdaptImpSampling.hpp"
#include "NonDCubature.hpp"
#include "NonDLHSSampling.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSparseGrid.hpp"
#include "ParallelLibrary.hpp"
#include "PecosApproximation.hpp"
#include "ProblemDescDB.hpp"
#include "RecastModel.hpp"
#include "SensAnalysisGlobal.hpp"
#include "SharedPecosApproxData.hpp"
#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_tabular_io.hpp"
#include "pecos_math_util.hpp"
#include "pecos_stat_util.hpp"

// #define DEBUG
// #define CONVERGENCE_DATA
// #define TEST_HESSIANS

namespace Dakota {

NonDExpansion::NonDExpansion(ProblemDescDB& problem_db,
                             ParallelLibrary& parallel_lib,
                             std::shared_ptr<Model> model)
    : NonD(problem_db, parallel_lib, model),
      expansionCoeffsApproach(-1),
      expansionBasisType(
          problem_db.get_short("method.nond.expansion_basis_type")),
      statsMetricMode(
          problem_db.get_short("method.nond.refinement_statistics_mode")),
      relativeMetric(
          problem_db.get_short("method.nond.convergence_tolerance_type") !=
          ABSOLUTE_CONVERGENCE_TOLERANCE),  // include DEFAULT,RELATIVE
      dimPrefSpec(problem_db.get_rv("method.nond.dimension_preference")),
      collocPtsSeqSpec(problem_db.get_sza("method.nond.collocation_points")),
      collocRatio(problem_db.get_real("method.nond.collocation_ratio")),
      termsOrder(1.),
      tensorRegression(problem_db.get_bool("method.nond.tensor_grid")),
      randomSeed(problem_db.get_int("method.random_seed")),
      fixedSeed(problem_db.get_bool("method.fixed_seed")),
      mlmfIter(0),
      multilevAllocControl(
          problem_db.get_short("method.nond.multilevel_allocation_control")),
      multilevDiscrepEmulation(
          problem_db.get_short("method.nond.multilevel_discrepancy_emulation")),
      kappaEstimatorRate(
          problem_db.get_real("method.nond.multilevel_estimator_rate")),
      gammaEstimatorScale(1.),
      numSamplesOnModel(0),
      numSamplesOnExpansion(
          problem_db.get_int("method.nond.samples_on_emulator")),
      nestedRules(false),
      piecewiseBasis(problem_db.get_bool("method.nond.piecewise_basis")),
      useDerivs(problem_db.get_bool("method.derivative_usage")),
      refineType(problem_db.get_short("method.nond.expansion_refinement_type")),
      refineControl(
          problem_db.get_short("method.nond.expansion_refinement_control")),
      refineMetric(
          problem_db.get_short("method.nond.expansion_refinement_metric")),
      softConvLimit(problem_db.get_ushort("method.soft_convergence_limit")),
      numUncertainQuant(0),
      maxRefineIterations(
          problem_db.get_sizet("method.nond.max_refinement_iterations")),
      maxSolverIterations(
          problem_db.get_sizet("method.nond.max_solver_iterations")),
      ruleNestingOverride(problem_db.get_short("method.nond.nesting_override")),
      ruleGrowthOverride(problem_db.get_short("method.nond.growth_override")),
      // Note: minimum VBD order for variance-controlled refinement is enforced
      //       in NonDExpansion::construct_{quadrature,sparse_grid}
      vbdOrderLimit(problem_db.get_ushort("method.nond.vbd_interaction_order")),
      covarianceControl(problem_db.get_short("method.nond.covariance_control"))
// For supporting construct_incremental_lhs():
// expansionSampleType(problem_db.get_string("method.expansion_sample_type"))
{
  check_dimension_preference(dimPrefSpec);
  initialize_counts();
  initialize_response_covariance();
  initialize_final_statistics();  // level mappings are available
}

NonDExpansion::NonDExpansion(
    unsigned short method_name, std::shared_ptr<Model> model,
    const ShortShortPair& approx_view, short exp_coeffs_approach,
    const RealVector& dim_pref, int seed, short refine_type,
    short refine_control, short covar_control, Real colloc_ratio,
    short rule_nest, short rule_growth, bool piecewise_basis, bool use_derivs)
    : NonD(method_name, model, approx_view),
      expansionCoeffsApproach(exp_coeffs_approach),
      expansionBasisType(Pecos::DEFAULT_BASIS),
      statsMetricMode(Pecos::DEFAULT_EXPANSION_STATS),
      relativeMetric(true),
      dimPrefSpec(dim_pref),
      collocRatio(colloc_ratio),
      termsOrder(1.),
      tensorRegression(false),
      randomSeed(seed),
      fixedSeed(false),
      mlmfIter(0),
      multilevAllocControl(DEFAULT_MLMF_CONTROL),
      multilevDiscrepEmulation(DEFAULT_EMULATION),
      kappaEstimatorRate(2.),
      gammaEstimatorScale(1.),
      numSamplesOnModel(0),
      numSamplesOnExpansion(0),
      nestedRules(false),
      piecewiseBasis(piecewise_basis),
      useDerivs(use_derivs),
      refineType(refine_type),
      refineControl(refine_control),
      refineMetric(Pecos::DEFAULT_METRIC),
      softConvLimit(3),
      numUncertainQuant(0),
      maxRefineIterations(SZ_MAX),
      maxSolverIterations(SZ_MAX),
      ruleNestingOverride(rule_nest),
      ruleGrowthOverride(rule_growth),
      vbdOrderLimit(0),
      covarianceControl(covar_control) {
  check_dimension_preference(dimPrefSpec);
  initialize_counts();

  // level mappings not yet available
  // (defer initialize_response_covariance() and initialize_final_statistics())
}

NonDExpansion::~NonDExpansion() {}

void NonDExpansion::initialize_counts() {
  const Variables& vars = iteratedModel->current_variables();
  const SizetArray& ac_totals = vars.shared_data().active_components_totals();

  // flag for combined var expansions which include non-probabilistic subset
  // (continuous only for now)
  allVars =
      (ac_totals[TOTAL_CDV] || ac_totals[TOTAL_CEUV] || ac_totals[TOTAL_CSV]);

  // override default definition in NonD ctor.  If there are any aleatory
  // variables, then we will sample on that subset for probabilistic stats.
  bool euv = (ac_totals[TOTAL_CEUV] || ac_totals[TOTAL_DEUIV] ||
              ac_totals[TOTAL_DEUSV] || ac_totals[TOTAL_DEURV]);
  bool auv = (ac_totals[TOTAL_CAUV] || ac_totals[TOTAL_DAUIV] ||
              ac_totals[TOTAL_DAUSV] || ac_totals[TOTAL_DAURV]);
  epistemicStats = (euv && !auv);
}

bool NonDExpansion::resize() {
  bool parent_reinit_comms = NonD::resize();

  check_dimension_preference(dimPrefSpec);
  initialize_counts();

  return parent_reinit_comms;
}

void NonDExpansion::derived_init_communicators(ParLevLIter pl_iter) {
  // this is redundant with Model recursions except for PCE coeff import case
  // iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // uSpaceModel, expansionSampler, and importanceSampler use
  // NoDBBaseConstructor, so no need to manage DB list nodes at this level
  if (expansionSampler)
    expansionSampler->init_communicators(pl_iter);
  else
    uSpaceModel->init_communicators(pl_iter, maxEvalConcurrency);

  if (importanceSampler) importanceSampler->init_communicators(pl_iter);
}

void NonDExpansion::derived_set_communicators(ParLevLIter pl_iter) {
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);

  // uSpaceModel, expansionSampler, and importanceSampler use
  // NoDBBaseConstructor, so no need to manage DB list nodes at this level
  if (expansionSampler)
    expansionSampler->set_communicators(pl_iter);
  else
    uSpaceModel->set_communicators(pl_iter, maxEvalConcurrency);

  if (importanceSampler) importanceSampler->set_communicators(pl_iter);
}

void NonDExpansion::derived_free_communicators(ParLevLIter pl_iter) {
  if (importanceSampler) importanceSampler->free_communicators(pl_iter);

  if (expansionSampler)
    expansionSampler->free_communicators(pl_iter);
  else
    uSpaceModel->free_communicators(pl_iter, maxEvalConcurrency);

  // this is redundant with Model recursions except for PCE coeff import case
  // iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}

void NonDExpansion::initialize_response_covariance() {
  // if diagonal only, utilize a vector (or sparse matrix) to optimize
  // both computational performance and memory footprint)
  bool refine_by_covar = (totalLevelRequests == 0);
  switch (covarianceControl) {
    case DEFAULT_COVARIANCE:  // assign context-specific default
      if (refine_by_covar)
        covarianceControl = FULL_COVARIANCE;
      else if (subIteratorFlag)
        covarianceControl = NO_COVARIANCE;
      else
        covarianceControl =
            (numFunctions > 10) ? DIAGONAL_COVARIANCE : FULL_COVARIANCE;
      break;
    case NO_COVARIANCE:  // won't happen since NO_COVARIANCE not exposed in spec
      if (refine_by_covar) {
        Cerr << "Warning: covariance required by refinement.  Adding diagonal "
             << "covariance terms." << std::endl;
        covarianceControl = DIAGONAL_COVARIANCE;
      }
      break;
  }
  // now that setting is defined, size the variance/covariance storage
  if (covarianceControl == FULL_COVARIANCE)
    respCovariance.shapeUninitialized(numFunctions);
  else if (covarianceControl == DIAGONAL_COVARIANCE)
    respVariance.sizeUninitialized(numFunctions);
}

void NonDExpansion::resolve_inputs(short& u_space_type, short& data_order) {
  // Expansion methods are currently ML or MF, not yet both (ENUMERATION_PREC)
  short mlmf_prec = (methodName == MULTIFIDELITY_POLYNOMIAL_CHAOS ||
                     methodName == MULTIFIDELITY_STOCH_COLLOCATION ||
                     methodName == MULTIFIDELITY_FUNCTION_TRAIN)
                        ? MULTIFIDELITY_PRECEDENCE
                        : MULTILEVEL_PRECEDENCE;
  bool err_flag = false, mf = (mlmf_prec == MULTIFIDELITY_PRECEDENCE),
       mf_greedy = (mf && multilevAllocControl == GREEDY_REFINEMENT);

  // define tie breaker for hierarchy of model forms versus resolution levels
  if (iteratedModel->surrogate_type() == "ensemble")
    iteratedModel->ensemble_precedence(
        mlmf_prec);  // don't reassign default keys

  // Check for suitable distribution types.
  // Note: prefer warning in Analyzer (active discrete ignored), but
  // RandomVariable type mapping must be defined...
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "\nError: active discrete variables are not currently supported "
         << "in NonDExpansion.\n";
    err_flag = true;
  }

  // check compatibility of refinement type with u-space type and MLMF settings
  switch (refineType) {
    case Pecos::H_REFINEMENT:  // override
      switch (u_space_type) {
        // case EXTENDED_U: // default; not user-selectable -> quiet default
        // reassign
        //   break;
        case ASKEY_U:
        case PARTIAL_ASKEY_U:  // non-default
          Cerr << "\nWarning: overriding transformation from ASKEY to "
                  "STD_UNIFORM "
               << "for h-refinement.\n"
               << std::endl;
          break;
        case STD_NORMAL_U:  // non-default
          Cerr << "\nWarning: overriding transformation from WIENER to "
                  "STD_UNIFORM "
               << "for h-refinement.\n"
               << std::endl;
          break;
      }

      u_space_type = STD_UNIFORM_U;
      piecewiseBasis = true;
      break;
    case Pecos::P_REFINEMENT:
      if (piecewiseBasis) {
        Cerr << "\nError: fixed order piecewise bases are incompatible with "
             << "p-refinement.\n";
        err_flag = true;
      }
      break;
    case Pecos::NO_REFINEMENT:
      if (mf_greedy) {
        Cerr << "Error: greedy integrated refinement of multifidelity "
                "expansions "
             << "requires a refinement specification for candidate "
                "generation.\n";
        err_flag = true;
      }
      break;
  }

  // Allow either ACTIVE or COMBINED with individual MF (default to COMBINED:
  // more important for relative, less so for absolute), but require COMBINED
  // for integrated MF.  Allow either sense for relativeMetric.
  switch (statsMetricMode) {
    case Pecos::NO_EXPANSION_STATS:  // should not happen
      Cerr << "Error: statsMetricMode definition required in NonDExpansion::"
           << "resolve_inputs()" << std::endl;
      err_flag = true;
      break;

    case Pecos::DEFAULT_EXPANSION_STATS:  // assign default
      statsMetricMode = (mf) ? Pecos::COMBINED_EXPANSION_STATS
                             :  // individual || integrated MF
                            Pecos::ACTIVE_EXPANSION_STATS;  // single fidelity
                                                            // || ML regression
      // can't propagate: shared_data_rep not yet constructed (DataFitSurrModel)
      break;

    case Pecos::ACTIVE_EXPANSION_STATS:  // ensure sanity of user spec
      // Disallow ACTIVE with integrated MLMF (greedy mlmfAllocControl)
      if (mf_greedy) {
        Cerr
            << "Error: combined expansion stats required for greedy integrated "
            << "multifidelity refinement." << std::endl;
        err_flag = true;
      }
      break;

    case Pecos::COMBINED_EXPANSION_STATS:  // ensure sanity of user spec
      if (!mf) {
        Cerr << "Error: combined expansion stats are only used for "
             << "multifidelity refinement." << std::endl;
        err_flag = true;
      }
      break;
  }
  // if individual MLMF with COMBINED, reorder loop in multifidelity_expansion()
  // to get a better initial reference for individual adaptation
  // > More consistent with greedy_mf to always do this, but seems less
  //   desirable to disconnect adaptations from reference builds
  // > Also requires clearing the starting expansions for recursive emulation

  // Enforce current support for recursive emulation
  if (multilevDiscrepEmulation == RECURSIVE_EMULATION && mf_greedy) {
    Cerr << "Error: recursive emulation not currently supported for greedy "
         << "integrated refinement\n       due to recursive recomputation "
         << "requirements.\n";
    err_flag = true;
  }

  if (err_flag) abort_handler(METHOD_ERROR);
}

void NonDExpansion::construct_cubature(
    std::shared_ptr<Iterator>& u_space_sampler,
    std::shared_ptr<Model> g_u_model, unsigned short cub_int_order) {
  // sanity checks: CUBATURE precluded since no grid anisotropy for adaptive
  // and very limited refinement opportunities for uniform/adaptive
  if (refineType) {
    Cerr << "Error: uniform/adaptive refinement of cubature grids not "
         << "supported." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  u_space_sampler = std::make_shared<NonDCubature>(g_u_model, cub_int_order);
}

void NonDExpansion::construct_quadrature(
    std::shared_ptr<Iterator>& u_space_sampler,
    std::shared_ptr<Model> g_u_model, unsigned short quad_order,
    const RealVector& dim_pref) {
  // sanity checks: no GSG for TPQ
  if (refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED) {
    Cerr << "Error: generalized option does not support adaptive refinement of "
         << "tensor grids." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL) {
    vbdFlag = true;
    vbdOrderLimit = 1;
  }

  // manage rule nesting override
  nestedRules = (ruleNestingOverride == Pecos::NESTED ||
                 (refineType && ruleNestingOverride != Pecos::NON_NESTED));

  short driver_mode = (false)  //(methodName == STOCH_COLLOCATION) // TO DO
                          ? Pecos::INTERPOLATION_MODE
                          : Pecos::INTEGRATION_MODE;

  u_space_sampler = std::make_shared<NonDQuadrature>(g_u_model, quad_order,
                                                     dim_pref, driver_mode);
}

void NonDExpansion::construct_quadrature(
    std::shared_ptr<Iterator>& u_space_sampler,
    std::shared_ptr<Model> g_u_model, unsigned short quad_order,
    const RealVector& dim_pref, int filtered_samples) {
  // sanity checks: only uniform refinement supported for probabilistic
  // collocation (regression using filtered tensor grids)
  if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
    Cerr << "Error: only uniform refinement is supported for regression with "
         << "the tensor_grid option." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  /*
  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL)
    { vbdFlag = true; vbdOrderLimit = 1; }

  // don't use nested rules for tensor regression since this could induce zeros
  // in the Vandermonde matrix (high order rules contain zeros for lower-order
  // polynomials), despite protection of m >= p+1
  nestedRules = (ruleNestingOverride == Pecos::NESTED ||
    (refineType && ruleNestingOverride != Pecos::NON_NESTED));
  */

  short driver_mode = (false)  //(methodName == STOCH_COLLOCATION) // TO DO
                          ? Pecos::INTERPOLATION_MODE
                          : Pecos::INTEGRATION_MODE;

  u_space_sampler = std::make_shared<NonDQuadrature>(
      g_u_model, quad_order, dim_pref, driver_mode, filtered_samples);
}

void NonDExpansion::construct_quadrature(
    std::shared_ptr<Iterator>& u_space_sampler,
    std::shared_ptr<Model> g_u_model, unsigned short quad_order,
    const RealVector& dim_pref, int sub_samples, int seed) {
  // sanity checks: only uniform refinement supported for probabilistic
  // collocation (regression using filtered tensor grids)
  if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
    Cerr << "Error: only uniform refinement is supported for regression with "
         << "the tensor_grid option." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  /*
  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL)
    { vbdFlag = true; vbdOrderLimit = 1; }

  // don't use nested rules for tensor regression since this could induce zeros
  // in the Vandermonde matrix (high order rules contain zeros for lower-order
  // polynomials), despite protection of m >= p+1
  nestedRules = (ruleNestingOverride == Pecos::NESTED ||
    (refineType && ruleNestingOverride != Pecos::NON_NESTED));
  */

  short driver_mode = (false)  //(methodName == STOCH_COLLOCATION) // TO DO
                          ? Pecos::INTERPOLATION_MODE
                          : Pecos::INTEGRATION_MODE;

  u_space_sampler = std::make_shared<NonDQuadrature>(
      g_u_model, quad_order, dim_pref, driver_mode, sub_samples, seed);
}

void NonDExpansion::construct_sparse_grid(
    std::shared_ptr<Iterator>& u_space_sampler,
    std::shared_ptr<Model> g_u_model, unsigned short ssg_level,
    const RealVector& dim_pref) {
  // enforce minimum required VBD control
  if (!vbdFlag && refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL) {
    vbdFlag = true;
    vbdOrderLimit = 1;
  }

  nestedRules = (ruleNestingOverride != Pecos::NON_NESTED);

  // tracking of unique product weights needed for PCE and SC standard modes
  // since both employ PolynomialApproximation::compute_numerical_moments(4).
  // Neither PCE nor SC require product wts for allVars mode, since moment
  // calculations must employ gauss_wts_1d.
  // Exception 1: allVars Nodal SC requires weights for total covariance()
  //              evaluation in VBD.
  // Exception 2: NonDIntegration::print_points_weights() needs weights for
  //              outputLevel > NORMAL_OUTPUT.
  bool nodal_vbd = (vbdFlag && methodName == STOCH_COLLOCATION &&
                    expansionCoeffsApproach != Pecos::HIERARCHICAL_SPARSE_GRID);
  bool track_wts = (!allVars || nodal_vbd || outputLevel > NORMAL_OUTPUT);

  short growth_rate;
  if (ruleGrowthOverride == Pecos::UNRESTRICTED ||
      refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED)
    // unstructured index set evolution: no motivation to restrict
    growth_rate = Pecos::UNRESTRICTED_GROWTH;
  // piecewise bases can be MODERATE *when* we distinguish INTERPOLATION_MODE
  // TO DO: comment out this else block when re-activating INTERPOLATION_MODE
  else if (piecewiseBasis)
    // no reason to match Gaussian precision, but restriction still useful:
    // use SLOW i=2l+1 since it is more natural for NEWTON_COTES,CLENSHAW_CURTIS
    // and is more consistent with UNRESTRICTED generalized sparse grids.
    growth_rate = Pecos::SLOW_RESTRICTED_GROWTH;
  else
    // INTEGRATION_MODE:   standardize on precision: i = 2m-1 = 2(2l+1)-1 = 4l+1
    // INTERPOLATION_MODE: standardize on number of interp pts: m = 2l+1
    growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH;
  // growth_rate = Pecos::SLOW_RESTRICTED_GROWTH; // sync with UQTk restricted

  short driver_mode = (false)  //(methodName == STOCH_COLLOCATION) // TO DO
                          ? Pecos::INTERPOLATION_MODE
                          : Pecos::INTEGRATION_MODE;

  u_space_sampler = std::make_shared<NonDSparseGrid>(
      g_u_model, ssg_level, dim_pref, expansionCoeffsApproach, driver_mode,
      growth_rate, refineControl, track_wts);
}

/*
BMA NOTE: If code is activated, need to instead use LHS, with refinement samples
void NonDExpansion::
construct_incremental_lhs(Iterator& u_space_sampler, Model& u_model,
                          int num_samples, int seed, const String& rng)
{
  // sanity checks
  if (num_samples <= 0) {
    Cerr << "Error: bad samples specification (" << num_samples << ") in "
         << "NonDExpansion::construct_incremental_lhs()." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // use default LHS sample_type for consistency with collocation support for
  // incremental_lhs but not incremental_random
  unsigned short sample_type = SUBMETHOD_DEFAULT; // default
  u_space_sampler.assign_rep(new NonDIncremLHSSampling(u_model, sample_type,
    num_samples, orig_seed, rng, ACTIVE), false);
}
*/

void NonDExpansion::initialize_u_space_model() {
  if (refineControl) {
    // communicate refinement metric to Pecos (determines internal
    // bookkeeping requirements for some PolyApproximation types)
    bool rel_mapping = false;
    if (totalLevelRequests) {
      for (size_t i = 0; i < numFunctions; ++i)
        if (!requestedRelLevels[i].empty() ||
            (respLevelTarget == RELIABILITIES &&
             !requestedRespLevels[i].empty())) {
          rel_mapping = true;
          break;
        }
    }

    switch (refineMetric) {
      case Pecos::DEFAULT_METRIC:
        if (totalLevelRequests)
          refineMetric = (rel_mapping) ? Pecos::MIXED_STATS_METRIC
                                       : Pecos::LEVEL_STATS_METRIC;
        else
          refineMetric = Pecos::COVARIANCE_METRIC;
        break;
        break;
      case Pecos::COVARIANCE_METRIC:
        if (totalLevelRequests)
          Cout
              << "Warning: refinement metric set to covariance.  Level mappings"
              << "\n        will not be used to guide adaptation." << std::endl;
        break;
      case Pecos::LEVEL_STATS_METRIC:
        if (totalLevelRequests)
          refineMetric = (rel_mapping) ? Pecos::MIXED_STATS_METRIC
                                       : Pecos::LEVEL_STATS_METRIC;
        else {
          Cerr
              << "Warning: refinement metric switched to covariance as no level"
              << "\n        mappings were provided." << std::endl;
          refineMetric = Pecos::COVARIANCE_METRIC;
        }
        break;
    }
  }

  // if all variables mode, init bookkeeping for the random variable subset
  if (allVars) {
    std::shared_ptr<SharedApproxData> shared_data_rep =
        uSpaceModel->shared_approximation().data_rep();

    BitArray random_vars_key(numContinuousVars);  // init to false
    assign_value(random_vars_key, true, startCAUV, numCAUV);
    shared_data_rep->random_variables_key(random_vars_key);
  }
}

void NonDExpansion::configure_expansion_orders(unsigned short exp_order,
                                               const RealVector& dim_pref,
                                               UShortArray& exp_orders) {
  // expansion_order defined for expansion_samples/regression
  if (exp_order == USHRT_MAX)
    exp_orders.clear();
  else
    Pecos::dimension_preference_to_anisotropic_order(
        exp_order, dim_pref, numContinuousVars, exp_orders);
}

void NonDExpansion::configure_pecos_options() {
  // Commonly used approx settings (e.g., order, outputLevel, useDerivs) are
  // passed via the DataFitSurrModel ctor chain.  Additional data needed by
  // {Orthog,Interp}PolyApproximation are passed in Pecos::
  // {Expansion,Basis,Regression}ConfigOptions.   Note: passing outputLevel
  // and useDerivs again is redundant with the DataFitSurrModel ctor.
  std::shared_ptr<SharedPecosApproxData> shared_data_rep =
      std::static_pointer_cast<SharedPecosApproxData>(
          uSpaceModel->shared_approximation().data_rep());
  Pecos::ExpansionConfigOptions ec_options(
      expansionCoeffsApproach, expansionBasisType,
      iteratedModel->correction_type(), multilevDiscrepEmulation, outputLevel,
      vbdFlag, vbdOrderLimit, refineControl, refineMetric, statsMetricMode,
      maxRefineIterations, maxSolverIterations, convergenceTol, softConvLimit);
  shared_data_rep->configuration_options(ec_options);
  Pecos::BasisConfigOptions bc_options(nestedRules, piecewiseBasis, true,
                                       useDerivs);
  shared_data_rep->configuration_options(bc_options);
}

void NonDExpansion::initialize_u_space_grid() {
  // if model resize is pending, defer initializing a potentially large grid
  if (iteratedModel->resize_pending()) { /* callResize = true; */
  } else {
    //
    // Note: not used by C3; Pecos restriction is appropriate (PCE/SC basis)
    //
    std::shared_ptr<SharedPecosApproxData> shared_data_rep =
        std::static_pointer_cast<SharedPecosApproxData>(
            uSpaceModel->shared_approximation().data_rep());
    std::shared_ptr<NonDIntegration> u_space_sampler_rep =
        std::static_pointer_cast<NonDIntegration>(
            uSpaceModel->subordinate_iterator());

    u_space_sampler_rep->initialize_grid(shared_data_rep->polynomial_basis());

    numSamplesOnModel =
        u_space_sampler_rep->maximum_evaluation_concurrency() /
        uSpaceModel->subordinate_model()->derivative_concurrency();
    // maxEvalConcurrency already updated for expansion samples and regression
    if (numSamplesOnModel)  // optional with default = 0
      maxEvalConcurrency *= numSamplesOnModel;
  }
}

void NonDExpansion::construct_expansion_sampler(
    unsigned short sample_type, const String& rng,
    unsigned short integration_refine, const IntVector& refine_samples,
    const String& import_approx_file, unsigned short import_approx_format,
    bool import_approx_active_only) {
  bool import_pts = false, exp_sampling = false;
  size_t i;
  if (!import_approx_file.empty())
    import_pts = exp_sampling = true;
  else if (totalLevelRequests)  //&& numSamplesOnExpansion) // catch as err
                                // below
    for (i = 0; i < numFunctions; ++i)
      if (requestedProbLevels[i].length() ||
          requestedGenRelLevels[i].length() ||
          (requestedRespLevels[i].length() &&
           respLevelTarget != RELIABILITIES)) {
        exp_sampling = true;
        break;
      }

  if (!exp_sampling) return;

  std::shared_ptr<NonD> exp_sampler_rep;
  if (import_pts) {
    RealMatrix x_samples;  // imports are always from user space
    // Analyzer::update_model_from_sample() currently updates only the active
    // continuous vars from an allSamples column --> pass numContinuousVars as
    // number of rows; import_build_active_only not currently used
    TabularIO::read_data_tabular(
        import_approx_file, "imported approx samples file", x_samples,
        numContinuousVars,
        import_approx_format);  //, import_build_active_only);
    numSamplesOnExpansion = x_samples.numCols();
    // transform to u space must follow runtime dist param updates,
    // so pass x_samples for now and transform at runtime
    exp_sampler_rep =
        std::make_shared<NonDSampling>(uSpaceModel, x_samples);  // u_samples);
    exp_sampler_rep->requested_levels(requestedRespLevels, requestedProbLevels,
                                      requestedRelLevels, requestedGenRelLevels,
                                      respLevelTarget, respLevelTargetReduce,
                                      cdfFlag, true);  // compute/print PDFs
  } else {
    if (!numSamplesOnExpansion) {  // sanity check for samples spec
      Cerr << "\nError: number of samples must be specified for numerically "
           << "evaluating statistics on a stochastic expansion." << std::endl;
      abort_handler(METHOD_ERROR);
    }

    // could use construct_lhs() except for non-default ALEATORY_UNCERTAIN
    // sampling mode.  Don't vary sampling pattern since we want to reuse
    // same sampling stencil for different design/epistemic vars or for
    // (goal-oriented) adaptivity.
    exp_sampler_rep = std::make_shared<NonDLHSSampling>(
        uSpaceModel, sample_type, numSamplesOnExpansion, first_seed(), rng,
        false,
        ALEATORY_UNCERTAIN);  //, ALL); *** HACK: ALEATORY_UNCERTAIN needed for
                              // aleatory stats but can we adopt the PCE view in
                              // cases of approx sample export (potentially
                              // resetting after numerical stats eval?)
    // expansionSampler->sampling_reset(numSamplesOnExpansion, true, false);

    // needs to precede exp_sampler_rep->requested_levels()
    exp_sampler_rep->final_moments_type(Pecos::NO_MOMENTS);  // suppress

    // publish level mappings to expansion sampler, but suppress reliability
    // moment mappings (performed locally within compute_statistics())
    RealVectorArray empty_rv_array;  // empty
    RealVectorArray& req_resp_levs = (respLevelTarget == RELIABILITIES)
                                         ? empty_rv_array
                                         : requestedRespLevels;
    exp_sampler_rep->requested_levels(
        req_resp_levs, requestedProbLevels, empty_rv_array,
        requestedGenRelLevels, respLevelTarget, respLevelTargetReduce, cdfFlag,
        false);  // suppress PDFs (managed locally)

    bool imp_sampling = false;
    if (integration_refine && respLevelTarget != RELIABILITIES)
      for (i = 0; i < numFunctions; ++i)
        if (requestedRespLevels[i].length()) {
          imp_sampling = true;
          break;
        }

    if (imp_sampling) {
      int ais_samples = 1000;  // context-specific default
      if (refine_samples.length() == 1)
        ais_samples = refine_samples[0];
      else if (refine_samples.length() > 1) {
        Cerr << "\nError (NonDExpansion): refinement_samples must be length "
             << "1 if specified." << std::endl;
        abort_handler(PARSE_ERROR);
      }
      // extreme values needed for defining bounds of PDF bins
      bool vary_pattern = true, track_extreme = pdfOutput;

      importanceSampler = std::make_shared<NonDAdaptImpSampling>(
          uSpaceModel, sample_type, ais_samples, first_seed(), rng,
          vary_pattern, integration_refine, cdfFlag, false, false,
          track_extreme);
      importanceSampler->output_level(outputLevel);
      std::static_pointer_cast<NonDAdaptImpSampling>(importanceSampler)
          ->requested_levels(req_resp_levs, empty_rv_array, empty_rv_array,
                             empty_rv_array, respLevelTarget,
                             respLevelTargetReduce, cdfFlag,
                             false);  // suppress PDFs (managed locally)
    }
  }
  // publish output verbosity
  exp_sampler_rep->output_level(outputLevel);
  // store rep inside envelope
  expansionSampler = exp_sampler_rep;
}

void NonDExpansion::core_run() {
  initialize_expansion();

  compute_expansion();  // nominal iso/aniso expansion from input spec
  if (refineType) {     //&& maxRefineIterations
    // post-process nominal expansion, defining reference stats for refinement
    // metric_roll_up(INTERMEDIATE_RESULTS); // not relevant for single-fidelity
    compute_statistics(INTERMEDIATE_RESULTS);
    if (outputLevel > SILENT_OUTPUT) print_results(Cout, INTERMEDIATE_RESULTS);

    refine_expansion();  // uniform/adaptive p-/h-refinement
  }

  compute_statistics(FINAL_RESULTS);
  // Note: print_results() called by Analyzer::post_run()

  finalize_expansion();
}

void NonDExpansion::initialize_expansion() {
  // IteratorScheduler::run_iterator() + Analyzer::initialize_run() ensure
  // initialization of Model mappings for iteratedModel, but local recursions
  // are not visible -> recur DataFitSurr +  ProbabilityTransform if needed.
  if (!uSpaceModel->mapping_initialized()) {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    /*bool var_size_changed =*/uSpaceModel->initialize_mapping(pl_iter);
    // if (var_size_changed) resize();
  }
  // Note: part of this occurs at DataFit build time (daceIterator -> g_u_model)
  // as well as evaluation of uSpaceModel by expansion sampler. Therefore, take
  // care to avoid redundancy using mappingInitialized flag.

  if (totalLevelRequests) initialize_level_mappings();  // size computed*Levels
  resize_final_statistics_gradients();  // finalStats ASV available at run time
  // transform_correlations();

  // now that data has flowed down at run-time from any higher level recursions
  // to iteratedModel, it must be propagated up through the local g_u_model and
  // uSpaceModel recursions (so they are correct when propagated back down).
  // There is no need to recur below iteratedModel.
  // RecastModel::update_from_model() had insufficient context to update
  // distribution parameters for variables that were not transformed, but
  // ProbabilityTransformModel::update_from_model() can now handle this.
  size_t layers = 2;  // PTModel, DFSModel
  // recur once (beyond layer 1) so that layer 2 pulls from iteratedModel
  uSpaceModel->update_from_subordinate_model(layers - 1);

  // if a sub-iterator, reset previous history (e.g. grid refinements) as needed
  if (subIteratorFlag) {  //&& numUncertainQuant && refineType) {
    auto u_space_sampler = uSpaceModel->subordinate_iterator();
    if (u_space_sampler)
      u_space_sampler
          ->reset();  // clear previous prior to next grid generate/eval
  }

  // set initialPtU which is used in this class for all-variables mode and local
  // sensitivity calculations, and by external classes for queries on the PCE
  // emulator model (e.g., NonDBayesCalibration).  In the case of design,
  // epistemic, and state vars, it captures the current values for this UQ
  // execution; for aleatory vars, it reflects the u-space mean values (which
  // are invariant in std distribution cases despite updates from above).
  initialPtU.size(numContinuousVars);
  if (allVars)
    uSpaceModel->trans_X_to_U(ModelUtils::continuous_variables(*iteratedModel),
                              initialPtU);
  RealVector u_means = uSpaceModel->multivariate_distribution().means();
  // const SharedVariablesData& svd
  //   = iteratedModel.current_variables().shared_data();
  for (size_t i = startCAUV; i < numCAUV; ++i)
    initialPtU[i] = u_means[i];  //[svd.cv_index_to_active_index(i)];

  // transform any points imported into expansionSampler from user space
  // into standardized space (must follow any dist param updates)
  if (expansionSampler && expansionSampler->method_name() == LIST_SAMPLING &&
      numUncertainQuant == 0) {
    std::shared_ptr<NonDSampling> exp_sampler_rep =
        std::static_pointer_cast<NonDSampling>(expansionSampler);
    exp_sampler_rep->transform_samples(*iteratedModel,
                                       *uSpaceModel);  // src,target
  }
}

void NonDExpansion::compute_expansion() {
#ifdef DERIV_DEBUG
  // numerical verification of analytic Jacobian/Hessian routines
  RealVector rdv_u;
  uSpaceModel->trans_X_to_U(ModelUtils::continuous_variables(*iteratedModel),
                            rdv_u);
  Pecos::ProbabilityTransformation& nataf =
      uSpaceModel->probability_transformation();
  nataf.verify_trans_jacobian_hessian(rdv_u);  //(rdv_x);
  nataf.verify_design_jacobian(rdv_u);
#endif  // DERIV_DEBUG

  auto u_space_sampler = uSpaceModel->subordinate_iterator();
  std::shared_ptr<NonD> u_space_sampler_rep =
      std::static_pointer_cast<NonD>(u_space_sampler);

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, j, rl_len, pl_len, bl_len, gl_len, total_i,
      cntr = 0, num_final_stats = final_asv.size(),
      num_final_grad_vars = final_dvv.size(),
      moment_offset = (finalMomentsType) ? 2 : 0;
  bool final_stat_grad_flag = false;
  for (i = 0; i < num_final_stats; ++i)
    if (final_asv[i] & 2) {
      final_stat_grad_flag = true;
      break;
    }

  // define ASV for u_space_sampler and expansion coeff/grad data flags
  ShortArray sampler_asv(numFunctions, 0);
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  size_t end_cauv = startCAUV + numCAUV;
  for (i = 0; i < numFunctions; ++i) {
    bool expansion_coeff_flag = false, expansion_grad_flag = false;
    if (totalLevelRequests) {
      rl_len = requestedRespLevels[i].length();
      pl_len = requestedProbLevels[i].length();
      bl_len = requestedRelLevels[i].length();
      gl_len = requestedGenRelLevels[i].length();
    } else
      rl_len = pl_len = bl_len = gl_len = 0;

    // map final_asv value bits into expansion_coeff_flag requirements
    total_i = moment_offset + rl_len + pl_len + bl_len + gl_len;
    for (j = 0; j < total_i; ++j)
      if (final_asv[cntr + j] & 1) {
        expansion_coeff_flag = true;
        break;
      }

    if (final_stat_grad_flag) {
      // moment grad flags manage requirements at a higher level
      // --> mapped into expansion value/grad flags at bottom
      bool moment1_grad = false, moment2_grad = false;
      // map final_asv gradient bits into moment grad requirements
      if (finalMomentsType) {
        if (final_asv[cntr++] & 2) moment1_grad = true;
        if (final_asv[cntr++] & 2) moment2_grad = true;
      }
      if (respLevelTarget == RELIABILITIES)
        for (j = 0; j < rl_len;
             ++j)  // dbeta/ds requires mu,sigma,dmu/ds,dsigma/ds
          if (final_asv[cntr + j] & 2) {
            moment1_grad = moment2_grad = expansion_coeff_flag = true;
            break;
          }
      cntr += rl_len + pl_len;
      for (j = 0; j < bl_len; ++j)  // dz/ds requires dmu/ds, dsigma/ds
        if (final_asv[cntr + j] & 2) {
          moment1_grad = moment2_grad = true;
          break;
        }
      cntr += bl_len + gl_len;

      // map moment grad requirements into expansion_{coeff,grad}_flag reqmts
      // (refer to *PolyApproximation::get_*_gradient() implementations)
      // aleatory vars requirements:
      //   mean grad:          coeff grads
      //   var  grad: coeffs & coeff grads
      // all vars requirements:
      //   mean grad: coeffs (nonrandom v),          coeff grads (random v)
      //   var  grad: coeffs (nonrandom v), coeffs & coeff grads (random v)
      if (allVars) {  // aleatory + design/epistemic
        size_t deriv_index, num_deriv_vars = final_dvv.size();
        for (j = 0; j < num_deriv_vars; ++j) {
          deriv_index = final_dvv[j] - 1;  // OK since we are in an "All" view
          // random variable
          if (deriv_index >= startCAUV && deriv_index < end_cauv) {
            if (moment1_grad) expansion_grad_flag = true;
            if (moment2_grad) expansion_grad_flag = expansion_coeff_flag = true;
          }
          // non-random variable
          else if (moment1_grad || moment2_grad)
            expansion_coeff_flag = true;
        }
      } else {  // aleatory expansion variables
        if (moment1_grad) expansion_grad_flag = true;
        if (moment2_grad) expansion_grad_flag = expansion_coeff_flag = true;
      }
    } else
      cntr += moment_offset + rl_len + pl_len + bl_len + gl_len;

    // map expansion_{coeff,grad}_flag requirements into sampler ASV and
    // Approximation settings
    if (expansion_coeff_flag) sampler_asv[i] |= 1;
    if (expansion_grad_flag || useDerivs) sampler_asv[i] |= 2;
    Approximation& approx_i = poly_approxs[i];
    approx_i.expansion_coefficient_flag(expansion_coeff_flag);
    approx_i.expansion_gradient_flag(expansion_grad_flag);
  }

  // If OUU/SOP (multiple calls to core_run()), an expansion constructed over
  // the full range of all variables does not need to be reconstructed on
  // subsequent calls.  However, an allVars construction over a trust region
  // needs rebuilding when the trust region is updated.  In the checks below,
  // all_approx detects any variable insertions or ASV omissions and
  // force_rebuild() manages variable augmentations.
  bool all_approx = false,
       dist_param_derivs =
           (uSpaceModel->query_distribution_parameter_derivatives() >
            NO_DERIVS);
  if (allVars && numUncertainQuant && !dist_param_derivs) {
    all_approx = true;
    // does sampler_asv contain content not evaluated previously
    const ShortArray& prev_asv = u_space_sampler->active_set_request_vector();
    for (i = 0; i < numFunctions; ++i)
      // bit-wise AND checks if each sampler_asv bit is present in prev_asv
      if ((prev_asv[i] & sampler_asv[i]) != sampler_asv[i]) {
        all_approx = false;
        break;
      }
  }
  if (!all_approx || uSpaceModel->force_rebuild()) {
    if (u_space_sampler_rep) {
      // Set the sampler ASV (defined from previous loop over numFunctions)
      ActiveSet sampler_set;
      sampler_set.request_vector(sampler_asv);

      // if required statistical sensitivities are not covered by All variables
      // mode for augmented design variables, then the simulations must evaluate
      // response sensitivities.
      bool sampler_grad = false;
      if (final_stat_grad_flag) {
        if (dist_param_derivs)
          uSpaceModel->activate_distribution_parameter_derivatives();
        if (allVars)
          sampler_grad = dist_param_derivs;
        else
          sampler_grad = true;
      }

      // Set the u_space_sampler DVV, managing different gradient modes & their
      // combinations.  The u_space_sampler's DVV may then be augmented for
      // correlations in NonD::set_u_to_x_mapping().  Sources for DVV content
      // include the model's continuous var ids and the final_dvv set by a
      // NestedModel.  In the latter case, NestedModel::derived_compute_response
      // maps top-level optimizer derivative vars to sub-iterator derivative
      // vars in NestedModel::set_mapping() and then sets this DVV within
      // finalStats using subIterator.response_results_active_set().
      if (useDerivs) {
        SizetMultiArrayConstView cv_ids =
            ModelUtils::continuous_variable_ids(*iteratedModel);
        if (sampler_grad) {  // merge cv_ids with final_dvv
          SizetSet merged_set;
          SizetArray merged_dvv;
          merged_set.insert(cv_ids.begin(), cv_ids.end());
          merged_set.insert(final_dvv.begin(), final_dvv.end());
          std::copy(merged_set.begin(), merged_set.end(), merged_dvv.begin());
          sampler_set.derivative_vector(merged_dvv);
        } else  // assign cv_ids
          sampler_set.derivative_vector(cv_ids);
      } else if (allVars &&
                 sampler_grad) {  // filter: retain only insertion tgts
        SizetArray filtered_final_dvv;
        for (i = 0; i < num_final_grad_vars; ++i) {
          size_t dvv_i = final_dvv[i];
          if (dvv_i > startCAUV && dvv_i <= end_cauv)
            filtered_final_dvv.push_back(dvv_i);
        }
        sampler_set.derivative_vector(filtered_final_dvv);
      } else if (sampler_grad)
        sampler_set.derivative_vector(final_dvv);
      else  // derivs not needed, but correct DVV len needed for MPI buffers
        sampler_set.derivative_vector(
            ModelUtils::continuous_variable_ids(*iteratedModel));

      // Build the orthogonal/interpolation polynomial approximations:
      u_space_sampler->active_set(sampler_set);
    }

    uSpaceModel->build_approximation();

    if (u_space_sampler_rep && final_stat_grad_flag && dist_param_derivs)
      uSpaceModel->deactivate_distribution_parameter_derivatives();
  }
}

void NonDExpansion::refine_expansion() {
  // --------------------------------------
  // Uniform/adaptive refinement approaches
  // --------------------------------------
  // DataMethod default for maxRefineIterations is SZ_MAX, indicating no user
  // spec.  Iteration counts are unconstrained in this case.
  size_t candidate, iter = 1;
  bool converged = (iter > maxRefineIterations),
       print_metric = (outputLevel > SILENT_OUTPUT);
  Real metric;

  pre_refinement();

  while (!converged) {
    Cout << "\n>>>>> Begin refinement iteration " << iter << ":\n";
    candidate = core_refinement(metric, false, print_metric);  // no revert
    if (candidate == SZ_MAX) {
      Cout
          << "\n<<<<< Refinement has saturated with no candidates available.\n";
      converged = true;
    } else {
      Cout << "\n<<<<< Refinement iteration " << iter << " completed: "
           << "convergence metric = " << metric << '\n';
      converged = (metric <= convergenceTol || ++iter > maxRefineIterations);
    }
  }

  post_refinement(metric);
}

void NonDExpansion::finalize_expansion() {
  ++numUncertainQuant;

  // IteratorScheduler::run_iterator() + Analyzer::initialize_run() ensure
  // finalization of Model mappings for iteratedModel, but local recursions
  // are not visible -> recur DataFitSurr +  ProbabilityTransform if needed.
  if (uSpaceModel->mapping_initialized()) {
    /*bool var_size_changed =*/uSpaceModel->finalize_mapping();
    // if (var_size_changed) resize();
  }
}

void NonDExpansion::pre_refinement() {
  // initialize refinement algorithms (if necessary)

  std::shared_ptr<Iterator> sub_iter_rep = uSpaceModel->subordinate_iterator();

  // now embedded in IncrementalSparseGridDriver::compute_grid():
  // nond_sparse->update_reference();

  switch (refineControl) {
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
      Cout << "\n>>>>> Initialization of generalized sparse grid sets.\n";
      std::static_pointer_cast<NonDSparseGrid>(sub_iter_rep)->initialize_sets();
      break;
  }
}

size_t NonDExpansion::core_refinement(Real& metric, bool revert,
                                      bool print_metric) {
  size_t candidate = 0;
  switch (refineControl) {
    case Pecos::UNIFORM_CONTROL:
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY: {
      // if refinement opportunities have saturated (e.g., increments have
      // reached max{Order,Rank} or previous cross validation indicated better
      // fit with lower order), no candidates will be generated for this model
      // key.
      if (!uSpaceModel->advancement_available()) {
        metric = 0.;
        return SZ_MAX;
      }

      RealVector stats_ref;
      if (revert) pull_reference(stats_ref);

      update_expansion();
      // combine expansions if necessary for metric computation:
      // Note: Multilevel SC overrides this fn to remove roll-up for Hier SC
      //       (its delta metrics can be computed w/o exp combination)
      metric_roll_up(REFINEMENT_RESULTS);

      // assess increment by computing refinement metric:
      // defer revert (pass false) -> simplifies best candidate tracking to
      // follow Note: covariance metric seems more self-consistent for
      // Sobol'-weighted aniso refinement, but allow final stats adaptation if
      // mappings are used
      switch (refineMetric) {
        case Pecos::COVARIANCE_METRIC:
          metric = compute_covariance_metric(false, print_metric);
          break;
        // case Pecos::MIXED_STATS_METRIC: // TO DO
        //   compute_mixed_metric(); [retire compute_final_stats_metric()]
        //   break;
        default:  // case Pecos::LEVEL_STATS_METRIC:
          metric = compute_level_mappings_metric(false, print_metric);
          break;
      }
      compute_statistics(
          REFINEMENT_RESULTS);  // augment delta metrics if needed
      if (print_metric)
        print_results(Cout, REFINEMENT_RESULTS);  // augment output
      pull_candidate(statsStar);  // pull compute_*_metric() + augmented stats

      if (revert) {
        pop_increment();
        push_reference(stats_ref);
      } else
        merge_grid();
      break;
    }
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:  // SSG only
      // Dimension adaptive refinement using generalized sparse grids.
      // > Start GSG from iso/aniso SSG: starting from scratch (w=0) is
      //   most efficient if fully nested; otherwise, unique points from
      //   lowest levels may not contribute (smolyak coeff = 0).
      // > Starting GSG from TPQ is conceptually straightforward but
      //   awkward in implementation (would need something like
      //   nond_sparse->ssg_driver->compute_tensor_grid()).
      // Returns best of several candidates for this level
      candidate = increment_sets(metric, revert, print_metric);
      break;
  }

  return candidate;
}

void NonDExpansion::post_refinement(Real& metric, bool reverted) {
  // finalize refinement algorithms (if necessary)
  switch (refineControl) {
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED: {
      bool converged_within_tol = (metric <= convergenceTol);
      finalize_sets(converged_within_tol, reverted);
      break;
    }
    case Pecos::UNIFORM_CONTROL:
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY:
      if (reverted && uSpaceModel->push_available())
        select_increment_candidate();
      break;
  }
}

void NonDExpansion::increment_grid(bool update_anisotropy) {
  switch (refineControl) {
    case Pecos::UNIFORM_CONTROL:
      switch (expansionCoeffsApproach) {
        case Pecos::QUADRATURE:
        case Pecos::CUBATURE:
        case Pecos::INCREMENTAL_SPARSE_GRID:
        case Pecos::HIERARCHICAL_SPARSE_GRID: {
          std::shared_ptr<NonDIntegration> nond_integration =
              std::static_pointer_cast<NonDIntegration>(
                  uSpaceModel->subordinate_iterator());
          nond_integration->increment_grid();
          break;
        }
        case Pecos::ORTHOG_LEAST_INTERPOLATION:  // case Pecos::SAMPLING:
          break;
        default:  // regression cases
          increment_order_and_grid();
          break;
      }
      break;
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL: {
      // Dimension adaptive refinement: define anisotropic preference
      // vector from total Sobol' indices, averaged over response fn set.
      std::shared_ptr<NonDIntegration> nond_integration =
          std::static_pointer_cast<NonDIntegration>(
              uSpaceModel->subordinate_iterator());
      if (update_anisotropy) {  // weight SSG to emphasize larger Sobol indices
        RealVector dim_pref;
        reduce_total_sobol_sets(dim_pref);
        nond_integration->increment_grid_preference(dim_pref);  // TPQ or SSG
      } else  // increment level while preserving current weighting / bounds
        nond_integration->increment_grid_preference();
      break;
    }
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY: {
      // Dimension adaptive refinement: define anisotropic weight vector
      // from min of spectral decay rates (PCE only) over response fn set.
      std::shared_ptr<NonDIntegration> nond_integration =
          std::static_pointer_cast<NonDIntegration>(
              uSpaceModel->subordinate_iterator());
      if (update_anisotropy) {  // weight SSG to emphasize slower decay
        RealVector aniso_wts;
        reduce_decay_rate_sets(aniso_wts);
        nond_integration->increment_grid_weights(aniso_wts);  // TPQ or SSG
      } else  // increment level while preserving current weighting / bounds
        nond_integration->increment_grid_weights();
      break;
    }
  }
}

void NonDExpansion::decrement_grid() {
  std::shared_ptr<NonDIntegration> nond_integration =
      std::static_pointer_cast<NonDIntegration>(
          uSpaceModel->subordinate_iterator());
  switch (expansionCoeffsApproach) {
    case Pecos::QUADRATURE:
    case Pecos::CUBATURE:
    case Pecos::INCREMENTAL_SPARSE_GRID:
    case Pecos::HIERARCHICAL_SPARSE_GRID:
      nond_integration->decrement_grid();
      break;
    case Pecos::ORTHOG_LEAST_INTERPOLATION:  // case Pecos::SAMPLING:
      break;
    default:  // regression cases
      decrement_order_and_grid();
      break;
  }
}

void NonDExpansion::push_increment() {
  increment_grid(false);  // don't recompute anisotropy

  switch (expansionCoeffsApproach) {
    case Pecos::INCREMENTAL_SPARSE_GRID:
    case Pecos::HIERARCHICAL_SPARSE_GRID: {
      std::shared_ptr<NonDIntegration> nond_integration =
          std::static_pointer_cast<NonDIntegration>(
              uSpaceModel->subordinate_iterator());
      nond_integration->push_grid_increment();
      break;
    }
  }

  uSpaceModel->push_approximation();  // uses reference in append_tensor_exp
}

void NonDExpansion::pop_increment() {
  // reverse order from update_expansion() / push_increment()
  // (allows grid increment to be queried while popping expansion data)
  uSpaceModel->pop_approximation(true);  // store increment to use in restore

  decrement_grid();

  switch (expansionCoeffsApproach) {
    case Pecos::INCREMENTAL_SPARSE_GRID:
    case Pecos::HIERARCHICAL_SPARSE_GRID: {
      std::shared_ptr<NonDIntegration> nond_integration =
          std::static_pointer_cast<NonDIntegration>(
              uSpaceModel->subordinate_iterator());
      nond_integration->pop_grid_increment();
      break;
    }
  }
}

void NonDExpansion::merge_grid() {
  switch (expansionCoeffsApproach) {
    case Pecos::INCREMENTAL_SPARSE_GRID:
    case Pecos::HIERARCHICAL_SPARSE_GRID: {
      std::shared_ptr<NonDIntegration> nond_integration =
          std::static_pointer_cast<NonDIntegration>(
              uSpaceModel->subordinate_iterator());
      nond_integration->merge_grid_increment();
      nond_integration->update_reference();
      break;
    }
      // case Pecos::QUADRATURE: case Pecos::CUBATURE: // no-op
  }
}

void NonDExpansion::configure_indices(size_t group, size_t form, size_t lev,
                                      short seq_type) {
  // Set active surrogate/truth models within the Hierarch,DataFit surrogates
  // (recursion from uSpaceModel to iteratedModel)
  // > group index is assigned based on step in model form/resolution sequence

  // preserve special values across type conversions
  unsigned short grp = (group == SZ_MAX) ? USHRT_MAX : (unsigned short)group,
                 frm = (form == SZ_MAX) ? USHRT_MAX : (unsigned short)form;
  Pecos::ActiveKey hf_key;
  hf_key.form_key(grp, frm, lev);

  if ((seq_type == Pecos::MODEL_FORM_1D_SEQUENCE && form == 0) ||
      (seq_type == Pecos::RESOLUTION_LEVEL_1D_SEQUENCE && lev == 0) ||
      !multilevDiscrepEmulation) {
    iteratedModel->surrogate_response_mode(BYPASS_SURROGATE);  // truth only
    uSpaceModel->active_model_key(hf_key);                     // one data set
    uSpaceModel
        ->resize_from_subordinate_model();  // recurs until hits aggregation
  } else {  // 3 data sets: HF, either LF or LF-hat, and discrep
    assign_surrogate_response_mode();
    Pecos::ActiveKey lf_key(hf_key.copy()), discrep_key;
    // using same child key for either LF or LF-hat
    lf_key.decrement_key(seq_type);  // seq_index defaults to 0
    // For ML/MF PCE/SC/FT, we both aggregate raw levels and apply a reduction
    discrep_key.aggregate_keys(lf_key, hf_key, Pecos::RAW_WITH_REDUCTION);
    uSpaceModel->active_model_key(discrep_key);
    uSpaceModel
        ->resize_from_subordinate_model();  // recurs until hits aggregation
  }
}

void NonDExpansion::assign_modes() {
  if (iteratedModel->surrogate_type() != "ensemble") {
    Cerr << "Error: multilevel/multifidelity expansions require an ensemble "
         << "model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // assign method-specific defaults for correction and discrepancy emulation
  if (iteratedModel->correction_type() == NO_CORRECTION)
    iteratedModel->correction_type(ADDITIVE_CORRECTION);
  if (multilevDiscrepEmulation == DEFAULT_EMULATION)
    multilevDiscrepEmulation =
        //(expBasisType==Pecos::HIERARCHICAL_INTERPOLANT) ? RECURSIVE_EMULATION
        //:
        DISTINCT_EMULATION;

  assign_surrogate_response_mode();
}

void NonDExpansion::assign_surrogate_response_mode() {
  // override default SurrogateModel::responseMode for purposes of setting
  // comms for the ordered Models within EnsembleSurrModel::set_communicators(),
  // which precedes mode updates in {multifidelity,multilevel}_expansion().

  // ML-MF {PCE,SC,FT} are based on model discrepancies, but multi-index cases
  // may evolve towards BYPASS_SURROGATE as sparse grids in model space will
  // manage QoI differences.
  // > AGGREGATED_MODEL_PAIR avoids decimation of data and can simplify methods,
  //   but requires additional discrepancy keys for high-low QoI combinations
  switch (multilevDiscrepEmulation) {
    case RECURSIVE_EMULATION:
      // consider disallowing for basis type Pecos::NODAL_INTERPOLANT
      iteratedModel->surrogate_response_mode(BYPASS_SURROGATE);
      break;
    case DISTINCT_EMULATION:
    case DEFAULT_EMULATION:
      // consider disallowing for basis type Pecos::HIERARCHICAL_INTERPOLANT
      // (HIERARCHICAL_INTERPOLANT approaches are already recursive...)
      iteratedModel->surrogate_response_mode(AGGREGATED_MODEL_PAIR);
      break;
  }
}

short NonDExpansion::initialize_costs(RealVector& cost,
                                      BitArray& model_cost_spec,
                                      SizetSizetPairArray& cost_md_indices) {
  ModelList& model_list = iteratedModel->subordinate_models(false);
  size_t m, num_mf = model_list.size();
  ModelLIter ml_it;
  cost_md_indices.resize(num_mf);
  for (m = 0, ml_it = model_list.begin(); ml_it != model_list.end();
       ++m, ++ml_it) {
    cost_md_indices[m].first = (*ml_it)->cost_metadata_index();
    cost_md_indices[m].second = (*ml_it)->current_response().metadata().size();
  }
  // we don't enforce cost data here since not all MF cases require it
  // --> defer test_cost() until downstream
  return query_cost(numSteps, sequenceType, cost, model_cost_spec,
                    cost_md_indices);
}

void NonDExpansion::compute_equivalent_cost(const SizetArray& N_l,
                                            const RealVector& cost) {
  if (cost.empty() || N_l.empty()) {
    equivHFEvals = 0.;
    return;
  }

  // compute the equivalent number of HF evaluations
  size_t l, num_l = N_l.size();
  switch (multilevDiscrepEmulation) {
    case RECURSIVE_EMULATION:
      for (l = 0; l < num_l; ++l)
        equivHFEvals += N_l[l] * cost[l];  // single model cost per level
      break;
    case DISTINCT_EMULATION:
      equivHFEvals = N_l[0] * cost[0];  // first level is single model cost
      for (l = 1; l < num_l; ++l)       // subsequent levels incur 2 model costs
        equivHFEvals += N_l[l] * (cost[l] + cost[l - 1]);
      break;
  }
  equivHFEvals /= cost[num_l - 1];  // normalize into equivalent HF evals
}

void NonDExpansion::multifidelity_expansion() {
  // costs are required for greedy refinement, optional otherwise
  bool greedy = (multilevAllocControl == GREEDY_REFINEMENT);
  if (greedy) test_cost(sequenceType, modelCostSpec, costMetadataIndices);

  // Separating reference + refinement into two loops accomplishes two things:
  // > allows refinement based on COMBINED_EXPANSION_STATS to have a more
  //   complete view of the rolled-up stats as level refinements begin
  // > more consistent flow + greater reuse between indiv/integrated refinement
  // > downside: recursive emulation requires update to ref expansions prior
  //   to initiating refinements for lev > 0
  // For greedy integrated refinement:
  // > Only generate combined{MultiIndex,ExpCoeffs,ExpCoeffGrads}; active
  //   multiIndex,expansionCoeff{s,Grads} remain at ref state (no roll up)
  multifidelity_reference_expansion();

  // Perform refinement (individual || integrated)
  if (greedy)
    multifidelity_integrated_refinement();  // refineType is required
  else
    multifidelity_individual_refinement();  // refineType is optional

  // promote combined expansion to active
  combined_to_active();
  // FINAL_RESULTS are computed / printed at end of virtual core_run()
}

void NonDExpansion::multifidelity_reference_expansion() {
  // clear any persistent state from previous run (e.g., for OUU)
  NLev.clear();  // zero sample counters
  mlmfIter = 0;  // zero iteration counter
  // remove default key (empty activeKey) since this interferes with
  // combine_approximation().  Also useful for ML/MF re-entrancy.
  uSpaceModel->clear_model_keys();
  // clearest to always use active stats for reference builds
  short orig_stats_mode = statsMetricMode;  // for restoration
  refinement_statistics_mode(Pecos::ACTIVE_EXPANSION_STATS);

  // Allow either model forms or discretization levels, but not both
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_1D_SEQUENCE),
       online_cost_recovery = (costSource && costSource != USER_COST_SPEC);
  // either lev varies and form is fixed, or vice versa:
  size_t form, lev;
  size_t& step = (multilev) ? lev : form;
  step = 0;
  if (multilev)
    form = secondaryIndex;
  else
    lev = secondaryIndex;
  std::shared_ptr<Iterator> dace_iterator = uSpaceModel->subordinate_iterator();

  // initial low fidelity/lowest discretization expansion
  configure_indices(step, form, lev, sequenceType);
  assign_specification_sequence();
  compute_expansion();  // nominal LF expansion from input spec
  RealVector accum_cost;
  SizetArray num_cost;
  if (online_cost_recovery) {
    accum_cost.size(numSteps);
    num_cost.assign(numSteps, 0);
    accumulate_online_cost(dace_iterator->all_responses(), step, accum_cost,
                           num_cost);
  }
  compute_statistics(INTERMEDIATE_RESULTS);

  bool print = (outputLevel > SILENT_OUTPUT);
  if (print) {
    Cout << "\n------------------------------------------------"
         << "\nMultifidelity UQ: low fidelity reference results"
         << "\n------------------------------------------------\n";
    print_results(Cout, INTERMEDIATE_RESULTS);
  }

  // loop over each of the discrepancy levels
  for (step = 1; step < numSteps; ++step) {
    // configure hierarchical model indices and activate key in data fit model
    configure_indices(step, form, lev, sequenceType);
    // advance to the next PCE/SC specification within the MF sequence
    increment_specification_sequence();

    // form the expansion for level i
    // Note: multi-batch parallel is impeded by the use of dace_iterator.run()
    // within DataFitSurrModel --> more challenging to queue batches.
    compute_expansion();  // nominal discrepancy expansion from input spec
    if (online_cost_recovery)
      accumulate_online_cost(dace_iterator->all_responses(), step, accum_cost,
                             num_cost);
    compute_statistics(INTERMEDIATE_RESULTS);

    if (print) {
      Cout << "\n-----------------------------------------------------"
           << "\nMultifidelity UQ: model discrepancy reference results"
           << "\n-----------------------------------------------------\n";
      print_results(Cout, INTERMEDIATE_RESULTS);
    }
  }
  if (online_cost_recovery)
    average_online_cost(accum_cost, num_cost, sequenceCost);

  // now aggregate expansions and report COMBINED_EXPANSION_STATS for cases
  // where the run will continue (individual/integrated refinement).
  // > If complete, then expansion combination + FINAL_RESULTS handled in
  //   higher level finalization operations.
  if (refineType) {  //&& maxRefineIterations
    // compute/print combined reference stats
    refinement_statistics_mode(Pecos::COMBINED_EXPANSION_STATS);
    metric_roll_up(INTERMEDIATE_RESULTS);  // combines approximations
    compute_statistics(INTERMEDIATE_RESULTS);
    if (print) {
      Cout << "\n----------------------------------------------------"
           << "\nMultifidelity UQ: statistics from combined expansion"
           << "\n----------------------------------------------------\n";
      print_results(Cout, INTERMEDIATE_RESULTS);
    }
  }

  refinement_statistics_mode(orig_stats_mode);  // restore
}

void NonDExpansion::multifidelity_individual_refinement() {
  // Allow either model forms or discretization levels, but not both
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_1D_SEQUENCE);
  // either lev varies and form is fixed, or vice versa:
  size_t form, lev;
  size_t& step = (multilev) ? lev : form;
  step = 0;
  if (multilev)
    form = secondaryIndex;
  else
    lev = secondaryIndex;

  bool print = (outputLevel > SILENT_OUTPUT);
  if (refineType) {  //&& maxRefineIterations
    // refine expansion for lowest fidelity/coarsest discretization
    configure_indices(step, form, lev, sequenceType);
    // assign_specification_sequence();
    refine_expansion();  // uniform/adaptive refinement
    metric_roll_up(INTERMEDIATE_RESULTS);
    compute_statistics(INTERMEDIATE_RESULTS);
    if (print) {
      Cout << "\n-------------------------------------------------"
           << "\nMultifidelity UQ: low fidelity refinement results"
           << "\n-------------------------------------------------\n";
      print_results(Cout, INTERMEDIATE_RESULTS);
    }

    // loop over each of the discrepancy levels
    for (step = 1; step < numSteps; ++step) {
      // configure hierarchical model indices and activate key in data fit model
      configure_indices(step, form, lev, sequenceType);
      // advance to the next PCE/SC specification within the MF sequence
      // increment_specification_sequence();

      // update discrepancy expansion since previous level has been refined
      if (multilevDiscrepEmulation ==
          RECURSIVE_EMULATION) {  //&&prev_lev_updated
        // update_expansion(); // no grid increment, no push

        // no new sim data, compute/use new synthetic data
        Cout << "\nRecompute step " << step + 1
             << " reference expansion due to "
             << "dependence on step " << step << " emulator.\n";
        uSpaceModel->formulation_updated(true);
        uSpaceModel->rebuild_approximation();
      }

      // refine the expansion for level i
      refine_expansion();  // uniform/adaptive refinement
      metric_roll_up(INTERMEDIATE_RESULTS);
      compute_statistics(INTERMEDIATE_RESULTS);
      if (print) {
        Cout << "\n------------------------------------------------------"
             << "\nMultifidelity UQ: model discrepancy refinement results"
             << "\n------------------------------------------------------\n";
        print_results(Cout, INTERMEDIATE_RESULTS);
      }
    }
  }

  // generate summary output across model sequence
  NLev.resize(numSteps);
  for (step = 0; step < numSteps; ++step) {
    configure_indices(step, form, lev, sequenceType);
    NLev[step] = uSpaceModel->approximation_data(0).points();  // first QoI
  }
  // cost specification is optional for multifidelity_expansion()
  if (!sequenceCost.empty())
    compute_equivalent_cost(NLev, sequenceCost);  // compute equiv # of HF evals
}

void NonDExpansion::multifidelity_integrated_refinement() {
  Cout << "\n-----------------------------------------------"
       << "\nMultifidelity UQ: initiating greedy competition"
       << "\n-----------------------------------------------\n";

  // Initialize again (or must propagate settings from mf_expansion())
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_1D_SEQUENCE);
  // either lev varies and form is fixed, or vice versa:
  size_t form, lev;
  size_t& step = (multilev) ? lev : form;
  if (multilev)
    form = secondaryIndex;
  else
    lev = secondaryIndex;

  // Initialize all levels.  Note: configure_indices() is used for completeness
  // (uSpaceModel->active_model_key(...) is sufficient for current grid
  // initialization operations).
  for (step = 0; step < numSteps; ++step) {
    configure_indices(step, form, lev, sequenceType);
    pre_refinement();
  }

  // Integrated MF refinement mirrors individual adaptive refinement in
  // refine_expansion() in using the max_refinement_iterations specification.
  // This differs from multilevel_regression(), which uses maxIterations and
  // potentially maxSolverIterations.
  size_t step_candidate, best_step, best_step_candidate;
  Real step_metric, best_step_metric = DBL_MAX;
  RealVector best_stats_star;
  bool print_metric = (outputLevel > SILENT_OUTPUT);
  while (best_step_metric > convergenceTol && mlmfIter < maxRefineIterations) {
    ++mlmfIter;
    Cout << "\n>>>>> Begin iteration " << mlmfIter << ": competing candidates "
         << "across " << numSteps << " sequence steps\n";

    // Generate candidates at each level
    best_step_metric = 0.;
    best_step = best_step_candidate = SZ_MAX;
    for (step = 0; step < numSteps; ++step) {
      Cout << "\n>>>>> Generating candidate(s) for sequence step " << step + 1
           << '\n';

      // configure hierarchical model indices and activate key in data fit model
      configure_indices(step, form, lev, sequenceType);

      // This returns the best/only candidate for the current level
      // Note: it must roll up contributions from all levels --> step_metric
      step_candidate = core_refinement(step_metric, true, print_metric);
      if (step_candidate == SZ_MAX)
        Cout << "\n<<<<< Sequence step " << step + 1
             << " has saturated with no refinement candidates available.\n";
      else {
        // core_refinement() normalizes level candidates based on the number of
        // required evaluations, which is sufficient for selection of the best
        // level candidate.  For selection among multiple level candidates, a
        // secondary normalization for relative level cost is required.
        Real l_cost = level_cost(step, sequenceCost);
        step_metric /= l_cost;
        Cout << "\n<<<<< Sequence step " << step + 1
             << " refinement metric = " << step_metric << " for step cost "
             << l_cost << '\n';
        // Assess candidate for best across all levels
        if (step_metric > best_step_metric) {
          best_step = step;
          best_step_candidate = step_candidate;
          best_step_metric = step_metric;
          best_stats_star = statsStar;
        }
      }
    }

    // permanently apply best increment and update references for next increment
    Cout << "\n<<<<< Iteration " << mlmfIter << " completed: ";
    if (best_step == SZ_MAX) {
      Cout << "no refinement selected.  Terminating iteration.\n";
      best_step_metric = 0.;  // kick out of loop
    } else {
      Cout << "selected refinement = sequence step " << best_step + 1
           << " candidate " << best_step_candidate + 1 << '\n';
      step = best_step;  // also updates form | lev
      configure_indices(step, form, lev, sequenceType);
      select_candidate(best_step_candidate);
      push_candidate(best_stats_star);  // update stats from best (no recompute)
      if (print_metric) print_results(Cout, INTERMEDIATE_RESULTS);
    }
  }

  // Perform final roll-up for each level and then combine levels
  NLev.resize(numSteps);
  bool reverted;
  for (step = 0; step < numSteps; ++step) {
    configure_indices(step, form, lev, sequenceType);
    reverted =
        (step != best_step);  // only candidate from best_step was applied
    post_refinement(best_step_metric, reverted);
    NLev[step] = uSpaceModel->approximation_data(0).points();  // first QoI
  }
  compute_equivalent_cost(NLev, sequenceCost);  // compute equiv # of HF evals
}

void NonDExpansion::multilevel_regression() {
  // Allow either model forms or discretization levels, but not both
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_1D_SEQUENCE);
  // either lev varies and form is fixed, or vice versa:
  size_t form, lev;
  size_t& step = (multilev) ? lev : form;
  if (multilev)
    form = secondaryIndex;
  else
    lev = secondaryIndex;

  Real eps_sq_div_2, sum_root_var_cost, estimator_var0 = 0., inv_kp1;
  if (multilevAllocControl == ESTIMATOR_VARIANCE) {  // cost estimates required
    test_cost(sequenceType, modelCostSpec, costMetadataIndices);
    inv_kp1 = 1. / (kappaEstimatorRate + 1.);
  }
  bool import_pilot,
      online_cost_recovery =
          (costSource && costSource != USER_COST_SPEC);  // online or mixed
  RealVector level_metrics(numSteps), accum_cost(numSteps);
  SizetArray num_cost;
  num_cost.assign(numSteps, 0);

  // virtual: base implementation clears keys, assigns stats type, et al.
  initialize_ml_regression(import_pilot);

  // Initialize NLev and load the pilot sample from user specification
  NLev.assign(numSteps, 0);
  SizetArray delta_N_l;
  if (collocPtsSeqSpec.empty() && collocRatio > 0.)
    infer_pilot_sample(/*collocRatio, */ numSteps, delta_N_l);
  else
    load_pilot_sample(collocPtsSeqSpec, numSteps, delta_N_l);

  /////////////////////////////
  // initial expansion build // (includes import and cost recovery)
  /////////////////////////////
  std::shared_ptr<Iterator> dace_iterator = uSpaceModel->subordinate_iterator();
  for (step = 0; step < numSteps; ++step) {
    configure_indices(step, form, lev, sequenceType);

    // Update solution control variable in uSpaceModel to support
    // DataFitSurrModel::consistent() logic
    if (import_pilot)
      uSpaceModel->update_from_subordinate_model();  // max depth

    NLev[step] += delta_N_l[step];  // update total samples for this step
    increment_sample_sequence(delta_N_l[step], NLev[step], step);
    if (step == 0 || import_pilot)
      compute_expansion();  // init + import + build; not recursive
    else
      update_expansion();  // just build; not recursive

    // Note: multi-batch parallel is impeded by the use of dace_iterator.run()
    // within DataFitSurrModel --> more challenging to queue batches.
    if (online_cost_recovery)
      accumulate_online_cost(dace_iterator->all_responses(), step, accum_cost,
                             num_cost);

    if (import_pilot) {  // update counts to include imported data
      NLev[step] = delta_N_l[step] =
          uSpaceModel->approximation_data(0).points();
      Cout << "Pilot count including import = " << delta_N_l[step] << "\n\n";
      // Trap zero samples as it will cause FPE downstream
      if (NLev[step] == 0) {  // no pilot spec, no import match
        Cerr << "Error: insufficient sample recovery for sequence step " << step
             << " in multilevel_regression()." << std::endl;
        abort_handler(METHOD_ERROR);
      }
    }

    switch (multilevAllocControl) {  // level_metrics depend on active config
      case ESTIMATOR_VARIANCE:
        aggregate_level_variance(level_metrics[step]);
        // MSE reference is ML MC aggregation for pilot(+import) sample:
        if (relativeMetric) estimator_var0 += level_metrics[step] / NLev[step];
        break;
      default:  // RIP_SAMPLING (ML PCE), RANK_SAMPLING (ML FT)
        sample_allocation_metric(level_metrics[step], 2.);
        break;
    }
  }
  if (online_cost_recovery)
    average_online_cost(accum_cost, num_cost, sequenceCost);

  switch (multilevAllocControl) {
    case ESTIMATOR_VARIANCE:
      sum_root_var_cost = 0.;
      for (step = 0; step < numSteps; ++step)
        sum_root_var_cost += std::pow(
            level_metrics[step] *
                std::pow(level_cost(step, sequenceCost), kappaEstimatorRate),
            inv_kp1);
      // eps^2 / 2 = var * relative factor || absolute factor
      eps_sq_div_2 = convergenceTol;
      if (relativeMetric) eps_sq_div_2 *= estimator_var0;
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Epsilon squared target = " << eps_sq_div_2 << '\n';
      compute_sample_increment(level_metrics, sequenceCost, sum_root_var_cost,
                               eps_sq_div_2, NLev, delta_N_l);
      break;
    default:  // RIP_SAMPLING (ML PCE), RANK_SAMPLING (ML FT)
      compute_sample_increment(level_metrics, NLev, delta_N_l);
      break;
  }
  ++mlmfIter;
  Cout << "\nML regression iteration " << mlmfIter << " sample increments:\n"
       << delta_N_l << std::endl;

  ///////////////////////////
  // refinement iterations //
  ///////////////////////////
  // now converge on sample counts per level (NLev)
  while (mlmfIter <= maxIterations && !zeros(delta_N_l)) {
    sum_root_var_cost = 0.;
    for (step = 0; step < numSteps; ++step) {
      configure_indices(step, form, lev, sequenceType);

      if (delta_N_l[step]) {
        NLev[step] += delta_N_l[step];  // update total samples for this step
        increment_sample_sequence(delta_N_l[step], NLev[step], step);
        // Note: import build data is not re-processed by append_expansion()
        append_expansion();
      }

      switch (multilevAllocControl) {
        case ESTIMATOR_VARIANCE: {
          Real& agg_var_l = level_metrics[step];
          if (delta_N_l[step]) aggregate_level_variance(agg_var_l);
          sum_root_var_cost +=
              std::pow(agg_var_l * std::pow(level_cost(step, sequenceCost),
                                            kappaEstimatorRate),
                       inv_kp1);
          break;
        }
        default:
          if (delta_N_l[step])  // else level metric same as previous iter
            sample_allocation_metric(level_metrics[step], 2.);
          break;
      }
    }

    switch (multilevAllocControl) {
      case ESTIMATOR_VARIANCE:
        compute_sample_increment(level_metrics, sequenceCost, sum_root_var_cost,
                                 eps_sq_div_2, NLev, delta_N_l);
        break;
      default:  // RIP_SAMPLING (ML PCE), RANK_SAMPLING (ML FT)
        compute_sample_increment(level_metrics, NLev, delta_N_l);
        break;
    }
    ++mlmfIter;
    Cout << "\nML regression iteration " << mlmfIter << " sample increments:\n"
         << delta_N_l << std::endl;
  }

  compute_equivalent_cost(NLev, sequenceCost);  // compute equiv # of HF evals
  finalize_ml_regression();  // annotated results computed/printed in core_run()
}

void NonDExpansion::initialize_ml_regression(bool& import_pilot) {
  mlmfIter = 0;

  // remove default key (empty activeKey) since this interferes with
  // combine_approximation().  Also useful for ML/MF re-entrancy.
  uSpaceModel->clear_model_keys();

  // all stats are stats for the active sequence step (not combined)
  refinement_statistics_mode(Pecos::ACTIVE_EXPANSION_STATS);

  // Multilevel variance aggregation requires independent sample sets
  std::shared_ptr<Iterator> u_sub_iter = uSpaceModel->subordinate_iterator();
  if (u_sub_iter != NULL)
    std::static_pointer_cast<Analyzer>(u_sub_iter)->vary_pattern(true);

  // Default (overridden in derived classes)
  import_pilot = false;
}

void NonDExpansion::finalize_ml_regression() {
  // combine level expansions and promote to active expansion:
  combined_to_active();
}

void NonDExpansion::accumulate_online_cost(const IntResponseMap& resp_map,
                                           size_t step, RealVector& accum_cost,
                                           SizetArray& num_cost) {
  // AGGREGATED_MODEL_PAIR: (NonDExpansion DISTINCT_EMULATION)
  // > a pair of Models is active
  // BYPASS_SURROGATE:      (NonDExpansion RECURSIVE_EMULATION)
  // > one Model is active

  // step = highest fidelity index so unwind to start from lowest active
  // fidelity based on active key data size

  using std::isfinite;
  const Pecos::ActiveKey& active_key = iteratedModel->active_model_key();
  unsigned short mf;
  Real cost;
  IntRespMCIter r_it;
  size_t key_index, start = 0, end, md_cntr = 0, md_index, md_index_m,
                    num_active = active_key.data_size(),
                    cost_index = step + 1 - num_active;

  for (key_index = 0; key_index < num_active; ++key_index, ++cost_index) {
    end = start + numFunctions;

    // Locate cost meta-data for ensemble member m through its model form
    // > see SurrogateModel::insert_metadata(): aggregated metadata is padded
    //   for any inactive models within the AGGREGATED_MODELS set
    mf = active_key.retrieve_model_form(key_index);
    const SizetSizetPair& cost_mdi = costMetadataIndices[mf];
    md_index_m = cost_mdi.first;
    if (md_index_m != SZ_MAX) {  // alternatively, if solnCntlCostMap key is 0.
      md_index = md_cntr + md_index_m;  // index into aggregated metadata
      for (r_it = resp_map.begin(); r_it != resp_map.end(); ++r_it) {
        const Response& resp = r_it->second;
        if (non_zero(resp.active_set_request_vector(), start, end)) {
          cost = resp.metadata(md_index);
          if (isfinite(cost)) {
            accum_cost[cost_index] += cost;
            ++num_cost[cost_index];
          }
        }
      }
    }

    start = end;
    md_cntr += cost_mdi.second;  // offset by size of metadata for model
  }
}

void NonDExpansion::select_candidate(size_t best_candidate) {
  // permanently apply best increment and update references for next increment
  switch (refineControl) {
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED: {
      // convert incoming candidate index to selected trial set
      std::shared_ptr<NonDSparseGrid> nond_sparse =
          std::static_pointer_cast<NonDSparseGrid>(
              uSpaceModel->subordinate_iterator());
      // active_mi index -> iterator mapping has not been invalidated
      // since candidate selection was previously deferred
      const std::set<UShortArray>& active_mi =
          nond_sparse->active_multi_index();
      std::set<UShortArray>::const_iterator best_cit = active_mi.begin();
      std::advance(best_cit, best_candidate);
      select_index_set_candidate(best_cit);
      break;
    }
    case Pecos::UNIFORM_CONTROL:
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY:
      select_increment_candidate();
      break;
  }

  // For distinct discrepancy, promotion of best candidate does not invalidate
  // coefficient increments for other levels, as they are separate functions.
  // Only the metric roll up must be updated when pushing existing expansion
  // increments.  For recursive discrepancy, however, all levels above the
  // selected candidate are invalidated.
  // if (multilevDiscrepEmulation == RECURSIVE_EMULATION)
  //  for (lev=best_lev+1; lev<numSteps; ++lev)
  //    uSpaceModel->clear_popped();
}

void NonDExpansion::select_index_set_candidate(
    std::set<UShortArray>::const_iterator cit_star) {
  std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>(
          uSpaceModel->subordinate_iterator());
  nond_sparse->update_sets(*cit_star);  // invalidates cit_star
  uSpaceModel->push_approximation();    // uses reference in append_tensor_exp
  nond_sparse->update_reference();
}

void NonDExpansion::select_increment_candidate() {
  // increment the grid and, if needed, the expansion order.
  // can ignore best_candidate (only one candidate for now).
  push_increment();
  merge_grid();  // adopt incremented state as new reference
}

void NonDExpansion::select_refinement_points(
    const RealVectorArray& candidate_samples, unsigned short batch_size,
    RealMatrix& best_samples) {
  Cerr << "Error: virtual select_refinement_points() not redefined by derived "
       << "class.\n       NonDExpansion does not support point selection."
       << std::endl;
  abort_handler(METHOD_ERROR);
}

void NonDExpansion::append_expansion() {
  // default implementation (may be overridden by derived classes)

  // Reqmts: numSamplesOnModel updated and propagated to uSpaceModel
  //         if necessary (PCE), increment_order_from_grid() has been called

  // Run uSpaceModel::daceIterator to generate numSamplesOnModel
  uSpaceModel->subordinate_iterator()->sampling_reset(numSamplesOnModel, true,
                                                      false);
  uSpaceModel->run_dace();
  // append new DACE pts and rebuild expansion
  uSpaceModel->append_approximation(true);
}

void NonDExpansion::append_expansion(const RealMatrix& samples,
                                     const IntResponseMap& resp_map) {
  // default implementation (may be overridden by derived classes)

  // increment the dataset
  numSamplesOnModel += resp_map.size();
  // utilize rebuild
  uSpaceModel->append_approximation(samples, resp_map, true);
}

/** Used for uniform refinement of regression-based PCE / FT. */
void NonDExpansion::increment_order_and_grid() {
  uSpaceModel->shared_approximation().increment_order();
  update_samples_from_order_increment();

  // update u-space sampler to use new sample count
  if (tensorRegression) {
    std::shared_ptr<NonDQuadrature> nond_quad =
        std::static_pointer_cast<NonDQuadrature>(
            uSpaceModel->subordinate_iterator());
    nond_quad->samples(numSamplesOnModel);
    if (nond_quad->mode() == RANDOM_TENSOR)
      nond_quad->increment_grid();  // increment dimension quad order
    nond_quad->update();
  }

  // assign number of total points in DataFitSurrModel
  update_model_from_samples();
}

/** Used for uniform de-refinement of regression-based PCE / FT. */
void NonDExpansion::decrement_order_and_grid() {
  uSpaceModel->shared_approximation().decrement_order();
  update_samples_from_order_decrement();

  // update u-space sampler to use new sample count
  if (tensorRegression) {
    std::shared_ptr<NonDQuadrature> nond_quad =
        std::static_pointer_cast<NonDQuadrature>(
            uSpaceModel->subordinate_iterator());
    nond_quad->samples(numSamplesOnModel);
    // if (nond_quad->mode() == RANDOM_TENSOR) ***
    //   nond_quad->decrement_grid(); // decrement dimension quad order
    nond_quad->update();
  }

  // assign number of total points in DataFitSurrModel
  update_model_from_samples();
}

void NonDExpansion::update_samples_from_order_increment() {
  Cerr << "Error: no base class implementation for NonDExpansion::"
       << "update_samples_from_order_increment()" << std::endl;
  abort_handler(METHOD_ERROR);
}

/** Default implementation: increment/decrement update process is identical */
void NonDExpansion::update_samples_from_order_decrement() {
  update_samples_from_order_increment();
}

void NonDExpansion::update_model_from_samples() {
  // for updates/rebuilds, zero out the lower bound (arises from honoring
  // an initial user spec alongside imports and min data requirements)
  // > now built in as part of of DataFitSurrModel::rebuild_global(), but
  //   multifidelity_reference_expansion() -> compute_expansion() also needs
  //   for sample updates (step > 0)  and resets (step = 0).
  uSpaceModel->subordinate_iterator()->sampling_reference(0);

  // enforce total pts (increment managed in DataFitSurrModel::rebuild_global())
  uSpaceModel->total_points(numSamplesOnModel);
  // std::shared_ptr<DataFitSurrModel> dfs_model =
  //   std::static_pointer_cast<DataFitSurrModel>(uSpaceModel->model_rep());
  // dfs_model->total_points(numSamplesOnModel);
}

/** leave sampler_set, expansion flags, and distribution parameter
    settings as set previously by compute_expansion(); there should be
    no need to update these for an expansion refinement. */
void NonDExpansion::update_expansion() {
  // Note: DIMENSION_ADAPTIVE_CONTROL_GENERALIZED does not utilize this fn

  increment_grid();  // recompute anisotropy

  if (uSpaceModel->push_available()) {  // defaults to false
    switch (expansionCoeffsApproach) {
        // case Pecos::QUADRATURE:              case Pecos::CUBATURE:
      case Pecos::INCREMENTAL_SPARSE_GRID:
      case Pecos::HIERARCHICAL_SPARSE_GRID: {
        std::shared_ptr<NonDIntegration> nond_int =
            std::static_pointer_cast<NonDIntegration>(
                uSpaceModel->subordinate_iterator());
        nond_int->push_grid_increment();
        break;
      }
        // no-op for SAMPLING, all REGRESSION cases
    }
    uSpaceModel->push_approximation();
  } else {
    switch (expansionCoeffsApproach) {
      case Pecos::QUADRATURE:
      case Pecos::CUBATURE:
      case Pecos::INCREMENTAL_SPARSE_GRID:
      case Pecos::HIERARCHICAL_SPARSE_GRID: {
        std::shared_ptr<NonDIntegration> nond_int =
            std::static_pointer_cast<NonDIntegration>(
                uSpaceModel->subordinate_iterator());
        nond_int
            ->evaluate_grid_increment();  // TPQ/Cub: not currently an increment
        break;
      }
    }
    switch (expansionCoeffsApproach) {
      case Pecos::QUADRATURE:
      case Pecos::CUBATURE:
        // replace the previous data and rebuild (prior to incremental support)
        uSpaceModel->update_approximation(true);
        break;
      case Pecos::INCREMENTAL_SPARSE_GRID:
      case Pecos::HIERARCHICAL_SPARSE_GRID:
        // append new data to the existing approximation and rebuild
        uSpaceModel->append_approximation(true);
        break;
      default:  // SAMPLING, REGRESSION: evaluate + append new data and rebuild
        // > if incremental unsupported, rebuild defaults to build from scratch.
        // > Note: DataFitSurrModel::rebuild_global() utilizes sampling_reset()
        //   and daceIterator.run() to define unstructured sample increment.
        uSpaceModel->rebuild_approximation();
        break;
    }
  }
}

void NonDExpansion::update_u_space_sampler(size_t sequence_index,
                                           const UShortArray& approx_orders) {
  std::shared_ptr<Iterator> sub_iter_rep = uSpaceModel->subordinate_iterator();
  int seed = NonDExpansion::seed_sequence(sequence_index);
  if (seed) sub_iter_rep->random_seed(seed);
  // replace w/ uSpaceModel->random_seed(seed)? -> u_space_sampler, shared
  // approx

  if (tensorRegression) {
    std::shared_ptr<NonDQuadrature> nond_quad =
        std::static_pointer_cast<NonDQuadrature>(sub_iter_rep);
    nond_quad->samples(numSamplesOnModel);
    if (nond_quad->mode() == RANDOM_TENSOR) {  // sub-sampling i/o filtering
      UShortArray dim_quad_order(numContinuousVars);
      for (size_t i = 0; i < numContinuousVars; ++i)
        dim_quad_order[i] = approx_orders[i] + 1;
      nond_quad->quadrature_order(
          dim_quad_order);  // update ref, enforce constr
    }
    nond_quad->update();  // sanity check on sizes, likely a no-op
  }
  // test for valid sampler for case of build data import (unstructured grid)
  else if (sub_iter_rep != NULL)  // enforce increment through sampling_reset()
    update_model_from_samples();
}

void NonDExpansion::refinement_statistics_mode(
    short stats_mode)  //, bool clear_bits)
{
  if (statsMetricMode != stats_mode) {
    statsMetricMode = stats_mode;

    /*
    // if poly_approxs share computed* trackers between active and combined,
    // then need to clear these trackers
    if (clear_bits) {
      std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
      for (size_t i=0; i<numFunctions; ++i)
        poly_approxs[i].clear_computed_bits();
    }

    // Changing stats rollup does *not* invalidate prodType{1,2}Coeffs since it
    // is defined only for expType{1,2}Coeffs (supporting delta_*() use cases),
    // but combined_to_active *does* for the active model index.
    // HIPA::combined_to_active() clears all prodType{1,2}Coeffs, such that
    // product_interpolants() will evaluate to false and new interpolants are
    // generated when needed (TO DO: any redundancy?).

    // For invalidation of current product interpolant data, we do not have
    // to have full knowledge of all model keys coming from NonDExpansion;
    // re-initializing the current model keys is enough.
    for (i=0; i<numFunctions; ++i)
      ((PecosApproximation*)poly_approxs[i].approx_rep())
        ->initialize_products(); // modify to enumerate all model keys; rename
                                 // existing to initialize_active_products()
    */
  }

  // propagate to DataFitSurrModel, SharedApproxData, etc.
  std::shared_ptr<SharedApproxData> shared_data_rep =
      uSpaceModel->shared_approximation().data_rep();
  shared_data_rep->refinement_statistics_mode(stats_mode);
}

void NonDExpansion::combined_to_active() {
  // default implementation

  // compute aggregate expansion (combined{MultiIndex,ExpCoeff{s,Grads}})
  // with limited stats support, retaining multiIndex,expansionCoeff{s,Grads}.
  uSpaceModel->combine_approximation();
  // migrate combined{MultiIndex,ExpCoeff{s,Grads}} to current active
  uSpaceModel->combined_to_active();
  // update approach for computing statistics; don't clear bits as
  // combined_to_active() can transfer bits from combined to active
  refinement_statistics_mode(Pecos::ACTIVE_EXPANSION_STATS);  //, false);
}

size_t NonDExpansion::increment_sets(Real& delta_star, bool revert,
                                     bool print_metric) {
  Cout << "\n>>>>> Begin evaluation of active index sets.\n";

  RealVector stats_ref;
  pull_reference(stats_ref);

  // Reevaluate the effect of every active set every time, since the reference
  // point for the surplus calculation changes (and the overlay should
  // eventually be inexpensive since each point set is only evaluated once).
  std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>(
          uSpaceModel->subordinate_iterator());
  const std::set<UShortArray>& active_mi = nond_sparse->active_multi_index();
  std::set<UShortArray>::const_iterator cit, cit_star = active_mi.end();
  Real delta;
  delta_star = -DBL_MAX;
  size_t index = 0, index_star = _NPOS;
  for (cit = active_mi.begin(); cit != active_mi.end(); ++cit, ++index) {
    // increment grid with current candidate
    Cout << "\n>>>>> Evaluating trial index set:\n" << *cit;
    nond_sparse->increment_set(*cit);
    if (uSpaceModel->push_available()) {  // has been active previously
      nond_sparse->push_set();
      uSpaceModel->push_approximation();
    } else {  // a new active set
      nond_sparse->evaluate_set();
      uSpaceModel->append_approximation(true);  // rebuild
    }

    // combine expansions if necessary for metric computation:
    // Note: Multilevel SC overrides this fn to remove roll-up for Hier SC
    //       (its delta metrics can be computed w/o exp combination)
    metric_roll_up(REFINEMENT_RESULTS);
    // assess increment by computing refinement metric:
    // defer revert (pass false) -> simplifies best candidate tracking to follow
    switch (refineMetric) {
      case Pecos::COVARIANCE_METRIC:
        delta = compute_covariance_metric(false, print_metric);
        break;
      // case Pecos::MIXED_STATS_METRIC: // TO DO
      //   compute_mixed_metric(); [retire compute_final_stats_metric()] break;
      default:  // case Pecos::LEVEL_STATS_METRIC:
        delta = compute_level_mappings_metric(false, print_metric);
        break;
    }
    compute_statistics(REFINEMENT_RESULTS);  // augment compute_*_metric()
    if (print_metric)
      print_results(Cout, REFINEMENT_RESULTS);  // augment output

    // normalize effect of increment based on cost (# of collocation pts).
    // Note: increment size is nonzero since growth restriction is precluded
    //       for generalized sparse grids.
    delta /= nond_sparse->increment_size();
    Cout << "\n<<<<< Trial set refinement metric = " << delta << '\n';
    // track best increment evaluated thus far
    if (delta > delta_star) {
      cit_star = cit;
      delta_star = delta;
      index_star = index;
      pull_candidate(statsStar);  // pull comp_*_metric() + augmented stats
    }

    // restore previous state (destruct order is reversed from construct order)
    uSpaceModel->pop_approximation(
        true);                     // store data for use in push,finalize
    nond_sparse->decrement_set();  // store data for use in push_set()
    if (revert || cit != --active_mi.end())  // else overwritten by push below
      push_reference(stats_ref);
  }
  Cout << "\n<<<<< Evaluation of active index sets completed.\n"
       << "\n<<<<< Index set selection:\n"
       << *cit_star;

  if (!revert) {  // permanently apply best increment and update references
    select_index_set_candidate(cit_star);  // invalidates cit_star
    push_candidate(statsStar);
    if (print_metric) print_results(Cout, INTERMEDIATE_RESULTS);
  }
  return index_star;
}

void NonDExpansion::finalize_sets(bool converged_within_tol, bool reverted) {
  Cout << "\n<<<<< Finalization of generalized sparse grid sets.\n";
  std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>(
          uSpaceModel->subordinate_iterator());
  // apply all remaining increments not previously selected
  bool output_sets = (outputLevel >= VERBOSE_OUTPUT);
  nond_sparse->finalize_sets(output_sets, converged_within_tol, reverted);
  uSpaceModel->finalize_approximation();
  nond_sparse->update_reference();  // for completeness
}

/** computes the default refinement metric based on change in respCovariance */
Real NonDExpansion::compute_covariance_metric(bool revert, bool print_metric) {
  // default implementation for use when direct (hierarchical) calculation
  // of increments is not available

  // Relative to computing the variance vector/covariance matrix, computing
  // mean values within compute_moments() adds little to no additional cost
  // > minor exception: PCE covariance does not require mean estimation but
  //   returning 1st coeff (and augmenting if all vars) is cheap
  // > {push,pull} of {reference,candidate} statistics to avoid recomputation
  //   means that prints of INTERMEDIATE results can be based on restoration
  //   of REFINEMENT results, so this simplifies the bridge between the two.
  // > Note: redundant moment estimations are protected by computed bits.

  Real scale;
  switch (covarianceControl) {
    case DIAGONAL_COVARIANCE: {
      // Note that the sense/sign of delta_resp_var is unimportant as it is only
      // used for the norm calculation --> more convenient to compute ref - new:
      RealVector resp_var_ref, delta_resp_var = respVariance;  // deep copy
      if (revert) resp_var_ref = respVariance;
      if (relativeMetric)
        scale = std::max(Pecos::SMALL_NUMBER_SQ, respVariance.normFrobenius());

      compute_moments();  // little to no additional cost (see above)
      // compute_covariance(); // minimal variance computation
      if (print_metric) print_covariance(Cout);
      delta_resp_var -= respVariance;  // compute change
      Real delta_norm = delta_resp_var.normFrobenius();

#ifdef DEBUG
      Cout << "resp_var_ref:\n"
           << resp_var_ref << "respVariance:\n"
           << respVariance << "norm of delta_resp_var = " << delta_norm
           << std::endl;
#endif  // DEBUG

      if (revert) respVariance = resp_var_ref;

      // For adaptation started from level = 0, reference covariance = 0.
      // Trap this and also avoid possible bogus termination from using absolute
      // change compared against relative conv tol.
      return (relativeMetric) ? delta_norm / scale : delta_norm;
      break;
    }
    case FULL_COVARIANCE: {
      // Note that the sense/sign of delta_resp_covar is unimportant as it is
      // only used for the norm calculation --> more convenient to compute ref -
      // new:
      RealSymMatrix resp_covar_ref,
          delta_resp_covar = respCovariance;  // deep copy
      if (revert) resp_covar_ref = respCovariance;
      if (relativeMetric)
        scale =
            std::max(Pecos::SMALL_NUMBER_SQ, respCovariance.normFrobenius());

      compute_moments();  // little to no additional cost (see above)
      compute_off_diagonal_covariance();
      // compute_covariance(); // minimal covariance computation
      if (print_metric) print_covariance(Cout);
      delta_resp_covar -= respCovariance;  // compute change
      Real delta_norm = delta_resp_covar.normFrobenius();

#ifdef DEBUG
      Cout << "resp_covar_ref:\n"
           << resp_covar_ref << "respCovariance:\n"
           << respCovariance << "norm of delta_resp_covar = " << delta_norm
           << std::endl;
#endif  // DEBUG

      if (revert) respCovariance = resp_covar_ref;

      // For adaptation started from level = 0, reference covariance = 0.
      // Trap this and also avoid possible bogus termination from using absolute
      // change compared against relative conv tol.
      return (relativeMetric) ? delta_norm / scale : delta_norm;
      break;
    }
    default:  // NO_COVARIANCE or failure to redefine DEFAULT_COVARIANCE
      return 0.;
      break;
  }
}

/** computes a "goal-oriented" refinement metric employing computed*Levels */
Real NonDExpansion::compute_level_mappings_metric(bool revert,
                                                  bool print_metric) {
  // default implementation for use when direct (hierarchical) calculation
  // of increments is not available

  // cache previous statistics
  size_t offset = 0;
  RealVector level_maps_ref;
  pull_level_mappings(level_maps_ref, offset);

  // compute/print new statistics
  compute_level_mappings();
  if (print_metric) print_level_mappings(Cout);
  RealVector level_maps_new;
  pull_level_mappings(level_maps_new, offset);

#ifdef DEBUG
  Cout << "level_maps_ref:\n"
       << level_maps_ref << "level_maps_new:\n"
       << level_maps_new << std::endl;
#endif  // DEBUG

  // sum up only the level mapping stats (don't mix with mean and variance due
  // to scaling issues).  Note: if the level mappings are of mixed type, then
  // would need to scale with a target value or measure norm of relative change.
  Real sum_sq = 0., scale_sq = 0.;
  size_t i, j, cntr = offset, num_lev_i;
  for (i = 0; i < totalLevelRequests; ++i, ++cntr) {
    // simple approach takes 2-norm of level mappings (no relative scaling),
    // which should be fine for mappings that are not of mixed type
    Real ref = level_maps_ref[cntr], delta = level_maps_new[cntr] - ref;
    if (relativeMetric) scale_sq += ref * ref;
    sum_sq += delta * delta;
  }

  if (revert) push_level_mappings(level_maps_ref, offset);

  // Risk of zero reference is reduced relative to covariance control, but not
  // eliminated. Trap this and also avoid possible bogus termination from using
  // absolute change compared against relative conv tol.
  if (relativeMetric) {
    Real scale = std::max(Pecos::SMALL_NUMBER, std::sqrt(scale_sq));
    return std::sqrt(sum_sq) / scale;
  } else
    return std::sqrt(sum_sq);
}

/*
// computes a "goal-oriented" refinement metric employing finalStatistics
Real NonDExpansion::
compute_final_statistics_metric(bool revert, bool print_metric)
{
  // default implementation for use when direct (hierarchical) calculation
  // of increments is not available

  // cache previous statistics
  RealVector final_stats_ref = finalStatistics.function_values(); // deep copy
  // *** Note: this requires that the reference includes FINAL_RESULTS,
  // *** which is not currently true (only INTERMEDIATE_RESULTS)

  // compute/print new statistics
  compute_statistics(FINAL_RESULTS); // no finalStats for REFINEMENT_RESULTS
  if (print_metric) print_results(Cout, FINAL_RESULTS);
  const RealVector& final_stats_new = finalStatistics.function_values();

#ifdef DEBUG
  Cout << "final_stats_ref:\n" << final_stats_ref
       << "final_stats_new:\n" << final_stats_new << std::endl;
#endif // DEBUG

  // sum up only the level mapping stats (don't mix with mean and variance due
  // to scaling issues).  Note: if the level mappings are of mixed type, then
  // would need to scale with a target value or measure norm of relative change.
  Real sum_sq = 0., scale_sq = 0.;
  size_t i, j, cntr = 0, num_lev_i, moment_offset = (finalMomentsType) ? 2 : 0;
  for (i=0; i<numFunctions; ++i) {

    // This modification can be used to mirror the metrics in Gerstner & Griebel
    // 2003.  However, their approach uses a hierarchical integral contribution
    // evaluation which is less prone to roundoff (and is refined to very tight
    // tolerances in the paper).
    //if (!finalMomentsType) { Cerr << "Error: "; abort_handler(METHOD_ERROR); }
    //sum_sq += std::pow(delta_final_stats[cntr], 2.); // mean
    //cntr += moment_offset + requestedRespLevels[i].length() +
    //  requestedProbLevels[i].length() + requestedRelLevels[i].length() +
    //  requestedGenRelLevels[i].length();

    // *** TO DO: support mixed metrics based on finalStats ASV
    cntr += moment_offset; // skip moments if final_stats
    num_lev_i = requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();
    for (j=0; j<num_lev_i; ++j, ++cntr) {
      Real ref  = final_stats_ref[cntr], delta = final_stats_new[cntr] - ref;
      if (relativeMetric) scale_sq += ref * ref;
      sum_sq += delta * delta;
    }
  }

  if (revert) finalStatistics.function_values(final_stats_ref);

  // Risk of zero reference is reduced relative to covariance control, but not
  // eliminated. Trap this and also avoid possible bogus termination from using
  // absolute change compared against relative conv tol.
  if (relativeMetric) {
    Real scale = std::max(Pecos::SMALL_NUMBER, std::sqrt(scale_sq));
    return std::sqrt(sum_sq) / scale;
  }
  else
    return std::sqrt(sum_sq);
}
*/

void NonDExpansion::compute_sample_increment(
    const RealVector& agg_var, const RealVector& cost, Real sum_root_var_cost,
    Real eps_sq_div_2, const SizetArray& N_l, SizetArray& delta_N_l) {
  // case ESTIMATOR_VARIANCE:

  // eps^2 / 2 target computed based on relative tolerance: total MSE = eps^2
  // which is equally apportioned (eps^2 / 2) among discretization MSE and
  // estimator variance (\Sum var_Y_l / NLev).  Since we do not know the
  // discretization error, we compute an initial estimator variance and then
  // seek to reduce it by a relative_factor <= 1.

  // We assume a functional dependence of estimator variance on NLev
  // for minimizing aggregate cost subject to an MSE error balance:
  //   Var(Q-hat) = sigma_Q^2 / (gamma NLev^kappa)
  // where Monte Carlo has gamma = kappa = 1.  To fit these parameters,
  // one approach is to numerically estimate the variance in the mean
  // estimator (alpha_0) from two sources:
  // > from variation across k folds for the selected CV settings
  // > from var decrease as NLev increases across iters

  // compute and accumulate variance of mean estimator from the set of
  // k-fold results within the selected settings from cross-validation:
  // Real cv_var_i = poly_approx_rep->
  //  cross_validation_solver().cv_metrics(MEAN_ESTIMATOR_VARIANCE);
  //  (need to make MultipleSolutionLinearModelCrossValidationIterator
  //   cv_iterator class scope)
  // To validate this approach, the actual estimator variance can be
  // computed and compared with the CV variance approximation (as for
  // traditional CV error plots, but predicting estimator variance
  // instead of L2 fit error).

  // update targets based on variance estimates
  Real new_N_l;
  size_t lev, num_lev = N_l.size();
  Real fact = std::pow(sum_root_var_cost / eps_sq_div_2 / gammaEstimatorScale,
                       1. / kappaEstimatorRate);
  for (lev = 0; lev < num_lev; ++lev) {
    new_N_l = std::pow(agg_var[lev] / level_cost(lev, cost),
                       1. / (kappaEstimatorRate + 1.)) *
              fact;
    delta_N_l[lev] = one_sided_delta(N_l[lev], new_N_l);
  }
}

void NonDExpansion::aggregate_level_variance(Real& agg_var_l) {
  // case ESTIMATOR_VARIANCE:
  // statsMetricMode remains as Pecos::ACTIVE_EXPANSION_STATS

  // control ML using aggregated variance across the vector of QoI
  // (alternate approach: target QoI with largest variance)
  agg_var_l = 0.;
  Real var_l;
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
    var_l = poly_approxs[qoi].variance();  // for active level
    agg_var_l += var_l;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Variance(" << "qoi " << qoi + 1 << ") = " << var_l << '\n';
  }
}

/** Default implementation redefined by Multilevel derived classes. */
void NonDExpansion::infer_pilot_sample(/*Real ratio, */ size_t num_steps,
                                       SizetArray& delta_N_l) {
  Cerr << "Error: no default implementation for infer_pilot_sample() used by "
       << "multilevel expansions." << std::endl;
  abort_handler(METHOD_ERROR);
}

void NonDExpansion::assign_specification_sequence() {
  Cerr << "Error: no default implementation for assign_specification_"
       << "sequence() used by multifidelity expansions." << std::endl;
  abort_handler(METHOD_ERROR);
}

/** Default implementation redefined by Multilevel derived classes. */
void NonDExpansion::increment_specification_sequence() {
  Cerr << "Error: no default implementation for increment_specification_"
       << "sequence() used by multifidelity expansions." << std::endl;
  abort_handler(METHOD_ERROR);
}

void NonDExpansion::increment_sample_sequence(size_t new_samp,
                                              size_t total_samp, size_t step) {
  Cerr << "Error: no default implementation for increment_sample_sequence() "
       << "defined for multilevel_regression()." << std::endl;
  abort_handler(METHOD_ERROR);
}

void NonDExpansion::sample_allocation_metric(Real& metric, Real power) {
  Cerr << "Error: no default implementation for sample_allocation_metric() "
       << "required for multilevel_regression()." << std::endl;
  abort_handler(METHOD_ERROR);
}

void NonDExpansion::compute_sample_increment(const RealVector& lev_metrics,
                                             const SizetArray& N_l,
                                             SizetArray& delta_N_l) {
  Cerr << "Error: no default implementation for compute_sample_increment() "
       << "defined for multilevel_regression()." << std::endl;
  abort_handler(METHOD_ERROR);
}

void NonDExpansion::reduce_total_sobol_sets(RealVector& avg_sobol) {
  // anisotropy based on total Sobol indices (univariate effects only) averaged
  // over the response fn set.  [Addition of interaction effects based on
  // individual Sobol indices would require a nonlinear index set constraint
  // within anisotropic sparse grids.]

  if (numFunctions > 1) {
    if (avg_sobol.empty())
      avg_sobol.size(numContinuousVars);  // init to 0
    else
      avg_sobol = 0.;
  }

  size_t i;
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (vbdOrderLimit)  // prevent print/export of uninitialized memory
      approx_i.clear_component_effects();
    else  // no order limit --> component used within total
      approx_i.compute_component_effects();
    approx_i.compute_total_effects();  // from scratch or using component

    // Note: response functions for which negligible variance is detected have
    // their totalSobolIndices assigned to zero.  This avoids corrupting the
    // aggregation, although the scaling that follows could be improved to
    // divide by the number of nonzero contributions (avg_sobol is currently
    // used only in a relative sense, so this is low priority).
    if (numFunctions > 1)
      avg_sobol += approx_i.total_sobol_indices();
    else
      avg_sobol = approx_i.total_sobol_indices();
  }
  // Trap case where there is no variance (yet) to attribute
  bool zero_sobol = true;
  for (i = 0; i < numContinuousVars; ++i)
    if (std::abs(avg_sobol[i]) > 0.) {
      zero_sobol = false;
      break;
    }

  if (zero_sobol) {
    avg_sobol.sizeUninitialized(0);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nSobol' indices not yet defined: no anisotropy detected"
           << std::endl;
  } else {
    if (numFunctions > 1) avg_sobol.scale(1. / (Real)numFunctions);
    // enforce lower bound on avg Sobol (disallow negative/zero)
    Real pref_tol = 1.e-2;  // TO DO
    for (i = 0; i < numContinuousVars; ++i)
      if (std::abs(avg_sobol[i]) < pref_tol) avg_sobol[i] = pref_tol;
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nUpdating anisotropy from average of total Sobol indices (lower"
           << " bounded by " << pref_tol << "):\n"
           << avg_sobol << std::endl;
  }
}

void NonDExpansion::reduce_decay_rate_sets(RealVector& min_decay) {
  // anisotropy based on linear approximation to coefficient decay rates for
  // each dimension as measured from univariate PCE terms.  In this case,
  // averaging tends to wash out the interesting anisotropy, especially if
  // some functions converge quickly with high rates.  Thus, it is more
  // appropriate to extract the minimum decay rates over the response fn set.

  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  // This context can be specific to PCE via Pecos
  std::shared_ptr<PecosApproximation> poly_approx_rep =
      std::static_pointer_cast<PecosApproximation>(
          poly_approxs[0].approx_rep());
  min_decay = poly_approx_rep->dimension_decay_rates();
  size_t i, j;
  for (i = 1; i < numFunctions; ++i) {
    poly_approx_rep = std::static_pointer_cast<PecosApproximation>(
        poly_approxs[i].approx_rep());
    const RealVector& decay_i = poly_approx_rep->dimension_decay_rates();
    for (j = 0; j < numContinuousVars; ++j)
      if (decay_i[j] < min_decay[j]) min_decay[j] = decay_i[j];
  }
  // Trap case where there is no decay (yet) to measure (mirror Sobol' logic)
  bool zero_decay = true;
  for (i = 0; i < numContinuousVars; ++i)
    if (std::abs(min_decay[i]) > 0.) {
      zero_decay = false;
      break;
    }

  if (zero_decay) {
    min_decay.sizeUninitialized(0);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nDecay rates not yet defined: no anisotropy detected"
           << std::endl;
  } else {
    // enforce lower bound on minimum decay (disallow negative/zero)
    Real decay_tol = 1.e-2;  // TO DO
    for (j = 0; j < numContinuousVars; ++j)
      if (min_decay[j] < decay_tol) min_decay[j] = decay_tol;
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nUpdating anisotropy from minimum decay rates (lower "
           << "bounded by " << decay_tol << "):\n"
           << min_decay << std::endl;
  }
}

void NonDExpansion::compute_active_diagonal_variance() {
  bool warn_flag = false;
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (size_t i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    Real& var_i = (covarianceControl == DIAGONAL_COVARIANCE)
                      ? respVariance[i]
                      : respCovariance(i, i);
    if (approx_i.expansion_coefficient_flag())
      var_i = (allVars) ? approx_i.variance(initialPtU) : approx_i.variance();
    else {
      warn_flag = true;
      var_i = 0.;
    }
  }
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDExpansion::"
         << "compute_covariance().\n         Zeroing affected variance terms."
         << std::endl;
}

void NonDExpansion::compute_active_off_diagonal_covariance() {
  size_t i, j;
  bool warn_flag = false;
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag())
      for (j = 0; j < i; ++j) {
        Approximation& approx_j = poly_approxs[j];
        if (approx_j.expansion_coefficient_flag())
          respCovariance(i, j) = (allVars)
                                     ? approx_i.covariance(initialPtU, approx_j)
                                     : approx_i.covariance(approx_j);
        else {
          warn_flag = true;
          respCovariance(i, j) = 0.;
        }
      }
    else {
      warn_flag = true;
      for (j = 0; j < i; ++j) respCovariance(i, j) = 0.;
    }
  }
  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDExpansion::"
         << "compute_off_diagonal_covariance().\n         Zeroing affected "
         << "covariance terms." << std::endl;
}

void NonDExpansion::compute_combined_diagonal_variance() {
  bool warn_flag = false;
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (size_t i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    Real& var_i = (covarianceControl == DIAGONAL_COVARIANCE)
                      ? respVariance[i]
                      : respCovariance(i, i);
    if (approx_i.expansion_coefficient_flag())
      var_i = (allVars) ? approx_i.combined_covariance(initialPtU, approx_i)
                        : approx_i.combined_covariance(approx_i);
    else {
      warn_flag = true;
      var_i = 0.;
    }
  }

  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDExpansion::"
         << "compute_combined_covariance().\n         Zeroing affected "
         << "covariance terms." << std::endl;
}

void NonDExpansion::compute_combined_off_diagonal_covariance() {
  size_t i, j;
  bool warn_flag = false;
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag())
      for (j = 0; j < i; ++j) {
        Approximation& approx_j = poly_approxs[j];
        if (approx_j.expansion_coefficient_flag())
          respCovariance(i, j) =
              (allVars) ? approx_i.combined_covariance(initialPtU, approx_j)
                        : approx_i.combined_covariance(approx_j);
        else {
          warn_flag = true;
          respCovariance(i, j) = 0.;
        }
      }
    else {
      warn_flag = true;
      for (j = 0; j <= i; ++j) respCovariance(i, j) = 0.;
    }
  }

  if (warn_flag)
    Cerr << "Warning: expansion coefficients unavailable in NonDExpansion::"
         << "compute_off_diagonal_combined_covariance().\n         Zeroing "
         << "affected covariance terms." << std::endl;
}

/** Calculate analytic and numerical statistics from the expansion and
    log results within final_stats for use in OUU. */
void NonDExpansion::compute_statistics(short results_state) {
  // restore variable settings following build/refine: supports local
  // sensitivities, expansion/importance sampling for all vars mode
  // (uses ALEATORY_UNCERTAIN sampling mode), and external uses of the
  // emulator model (emulator-based inference).
  // ModelUtils::continuous_variables(uSpaceModel, initialPtU);

  switch (results_state) {
    case REFINEMENT_RESULTS:
      // compute_{covariance,level_mapping,final_statistics}_metric() performs
      // the necessary computations for resolving delta.norm()

      // mirror requirements for additional diagnostics in print_results()
      // switch (refineControl) {
      // case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
      // print_smolyak_multi_index() requires nothing additional
      // break;
      // case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY:
      //  dimension_decay_rates() output only requires coefficients + basis
      //  norms
      // break;
      // case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
      //  print_sobol_indices() requires compute_sobol_indices(), but
      //  increment_grid() --> reduce_total_sobol_sets() takes care of this
      // compute_sobol_indices();
      // break;
      //}
      break;
    case INTERMEDIATE_RESULTS:
      switch (refineMetric) {
        case Pecos::DEFAULT_METRIC:  // possible for multifidelity_expansion()
          compute_moments();
          if (totalLevelRequests) {
            if (allVars)
              ModelUtils::continuous_variables(*uSpaceModel,
                                               initialPtU);  // see top
            compute_level_mappings();
          }
          break;
        case Pecos::COVARIANCE_METRIC:
          compute_moments();  // no additional cost (mean,variance reused)
          if (covarianceControl == FULL_COVARIANCE)
            compute_off_diagonal_covariance();
          break;
        case Pecos::MIXED_STATS_METRIC:
          if (allVars)
            ModelUtils::continuous_variables(*uSpaceModel,
                                             initialPtU);  // see top
          compute_moments();
          compute_level_mappings();
          break;
        case Pecos::LEVEL_STATS_METRIC:
          if (allVars)
            ModelUtils::continuous_variables(*uSpaceModel,
                                             initialPtU);  // see top
          compute_level_mappings();
          break;
      }
      break;
    case FINAL_RESULTS:
      ModelUtils::continuous_variables(*uSpaceModel,
                                       initialPtU);  // see top comment
      // -----------------------------
      // Calculate analytic statistics: includes derivs + finalStats updating
      // -----------------------------
      compute_analytic_statistics();  // stats derived from exp coeffs
      // ------------------------------
      // Calculate numerical statistics: with finalStats updating (no derivs)
      // ------------------------------
      compute_numerical_statistics();  // stats from sampling on expansion

      // update finalStatistics and archive results
      update_final_statistics();  // augments updates embedded above
      if (resultsDB
              .active()) {  // archive the active variables with the results
        resultsDB.insert(
            run_identifier(), resultsNames.cv_labels,
            ModelUtils::continuous_variable_labels(*iteratedModel));
        resultsDB.insert(run_identifier(), resultsNames.fn_labels,
                         ModelUtils::response_labels(*iteratedModel));
      }
      archive_moments();
      archive_coefficients();
      if (vbdFlag) archive_sobol_indices();
      break;
  }
}

void NonDExpansion::compute_level_mappings() {
  // for use with incremental results states (combines code from
  // compute_analytic_statistics() and compute_numerical_statistics(),
  // which support the final results state)

  // start with numerical, then overlay analytic below
  compute_numerical_level_mappings();

  // flags for limiting unneeded computation (matched in print_results())
  bool combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS),
       z_to_beta = (respLevelTarget == RELIABILITIES);

  // loop over response fns and compute/store analytic stats/stat grads
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  Real mu, var, sigma, p, z_bar, beta_bar;
  size_t i, j, rl_len, pl_len, bl_len,
      cntr = 0, moment_offset = (finalMomentsType) ? 2 : 0;

  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];

    rl_len = requestedRespLevels[i].length();
    pl_len = requestedProbLevels[i].length();
    bl_len = requestedRelLevels[i].length();

    cntr += moment_offset;

    // Note: corresponding logic in NonDExpansion::compute_expansion() defines
    // expansionCoeffFlag as needed to support final data requirements.
    // If not full stats, suppress secondary moment calculations.
    bool moments_flag = false;
    if (z_to_beta) {
      size_t offset = cntr;
      for (j = 0; j < rl_len; ++j)
        if (final_asv[j + offset] & 1) {
          moments_flag = true;
          break;
        }
    }
    if (!moments_flag) {
      size_t offset = cntr + rl_len + pl_len;
      for (j = 0; j < bl_len; ++j)
        if (final_asv[j + offset] & 1) {
          moments_flag = true;
          break;
        }
    }
    if (moments_flag) {
      if (allVars)
        approx_i.compute_moments(initialPtU, false, combined_stats);
      else
        approx_i.compute_moments(false, combined_stats);

      const RealVector& moments = approx_i.moments();  // virtual
      mu = moments[0];
      var = moments[1];  // Pecos provides central moments

      if (var >= 0.)
        sigma = std::sqrt(var);
      else {  // negative variance can happen with SC on sparse grids
        Cerr << "Warning: stochastic expansion variance is negative in "
             << "computation of std deviation.\n         Setting std "
             << "deviation to zero." << std::endl;
        sigma = 0.;
      }
    }

    if (z_to_beta) {
      for (j = 0; j < rl_len; ++j, ++cntr)
        if (final_asv[cntr] & 1) {
          z_bar = requestedRespLevels[i][j];
          if (!Pecos::is_small(sigma))
            computedRelLevels[i][j] =
                (cdfFlag) ? (mu - z_bar) / sigma : (z_bar - mu) / sigma;
          else
            computedRelLevels[i][j] =
                ((cdfFlag && mu <= z_bar) || (!cdfFlag && mu > z_bar))
                    ? -Pecos::LARGE_NUMBER
                    : Pecos::LARGE_NUMBER;
        }
    } else
      cntr += rl_len;

    cntr += pl_len;

    for (j = 0; j < bl_len; ++j, ++cntr)
      if (final_asv[cntr] & 1) {
        beta_bar = requestedRelLevels[i][j];
        computedRespLevels[i][j + pl_len] =
            (cdfFlag) ? mu - beta_bar * sigma : mu + beta_bar * sigma;
      }

    cntr += requestedGenRelLevels[i].length();
  }
}

void NonDExpansion::compute_numerical_level_mappings() {
  // for use with incremental results states (combines code from
  // compute_analytic_statistics() and compute_numerical_statistics(),
  // which support the final results state)

  // perform sampling on expansion for any numerical level mappings
  RealVector exp_sampler_stats;
  RealVectorArray imp_sampler_stats;
  RealRealPairArray min_max_fns;
  ShortArray sampler_asv;
  define_sampler_asv(sampler_asv);
  if (non_zero(sampler_asv)) {
    run_sampler(sampler_asv, exp_sampler_stats);
    refine_sampler(imp_sampler_stats, min_max_fns);
  }
  std::shared_ptr<NonDSampling> exp_sampler_rep =
      std::static_pointer_cast<NonDSampling>(expansionSampler);

  // flags for limiting unneeded computation (matched in print_results())
  bool z_to_beta = (respLevelTarget == RELIABILITIES),
       imp_sampling = bool(importanceSampler);

  // loop over response fns and compute/store analytic stats/stat grads
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  size_t i, j, rl_len, pl_len, bl_len, gl_len,
      cntr = 0, sampler_cntr = 0, moment_offset = (finalMomentsType) ? 2 : 0,
      sampler_moment_offset = 0;
  if (exp_sampler_rep != NULL && exp_sampler_rep->final_moments_type())
    sampler_moment_offset = 2;

  for (i = 0; i < numFunctions; ++i) {
    rl_len = requestedRespLevels[i].length();
    pl_len = requestedProbLevels[i].length();
    bl_len = requestedRelLevels[i].length();
    gl_len = requestedGenRelLevels[i].length();

    cntr += moment_offset;
    sampler_cntr += sampler_moment_offset;

    if (z_to_beta)
      cntr += rl_len;  // don't increment sampler_cntr
    else {
      for (j = 0; j < rl_len; ++j, ++cntr, ++sampler_cntr) {
        if (final_asv[cntr] & 1) {
          Real p = (imp_sampling) ? imp_sampler_stats[i][j]
                                  : exp_sampler_stats[sampler_cntr];
          if (respLevelTarget == PROBABILITIES)
            computedProbLevels[i][j] = p;
          else if (respLevelTarget == GEN_RELIABILITIES)
            computedGenRelLevels[i][j] =
                -Pecos::NormalRandomVariable::inverse_std_cdf(p);
        }
      }
    }

    for (j = 0; j < pl_len; ++j, ++cntr, ++sampler_cntr)
      if (final_asv[cntr] & 1)
        computedRespLevels[i][j] = exp_sampler_stats[sampler_cntr];

    cntr += bl_len;  // don't increment sampler_cntr

    for (j = 0; j < gl_len; ++j, ++cntr, ++sampler_cntr)
      if (final_asv[cntr] & 1)
        computedRespLevels[i][j + pl_len + bl_len] =
            exp_sampler_stats[sampler_cntr];
  }
}

void NonDExpansion::compute_moments() {
  // for use with incremental results states

  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  bool combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
  for (size_t i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag()) {
      if (allVars)
        approx_i.compute_moments(initialPtU, false, combined_stats);
      else
        approx_i.compute_moments(false, combined_stats);

      // extract variance (Pecos provides central moments)
      Real var =
          (combined_stats) ? approx_i.combined_moment(1) : approx_i.moment(1);
      if (covarianceControl == DIAGONAL_COVARIANCE)
        respVariance[i] = var;
      else if (covarianceControl == FULL_COVARIANCE)
        respCovariance(i, i) = var;
    }
  }
}

void NonDExpansion::compute_sobol_indices() {
  // for use with incremental results states

  if (!vbdFlag) return;

  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (size_t i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag()) {
      approx_i.compute_component_effects();  // main or main+interaction
      approx_i.compute_total_effects();      // total
    }
  }
}

void NonDExpansion::compute_analytic_statistics() {
  // for use with FINAL_RESULTS state

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, j, k, rl_len, pl_len, bl_len, gl_len,
      cntr = 0, num_final_grad_vars = final_dvv.size(), total_offset,
      moment_offset = (finalMomentsType) ? 2 : 0;

  // flags for limiting unneeded computation (matched in print_results())
  bool combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS),
       local_grad_stats = (!subIteratorFlag && outputLevel >= NORMAL_OUTPUT);

  if (local_grad_stats && expGradsMeanX.empty())
    expGradsMeanX.shapeUninitialized(numContinuousVars, numFunctions);

  // loop over response fns and compute/store analytic stats/stat grads
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  Real mu, var, sigma, beta, z;
  RealVector mu_grad, sigma_grad, final_stat_grad;
  for (i = 0; i < numFunctions; ++i) {
    if (totalLevelRequests) {
      rl_len = requestedRespLevels[i].length();
      pl_len = requestedProbLevels[i].length();
      bl_len = requestedRelLevels[i].length();
      gl_len = requestedGenRelLevels[i].length();
    } else
      rl_len = pl_len = bl_len = gl_len = 0;

    Approximation& approx_i = poly_approxs[i];

    // Note: corresponding logic in NonDExpansion::compute_expansion() defines
    // expansionCoeffFlag as needed to support final data requirements.
    // If not full stats, suppress secondary moment calculations.
    if (approx_i.expansion_coefficient_flag()) {
      if (allVars)
        approx_i.compute_moments(initialPtU, true, combined_stats);
      else
        approx_i.compute_moments(true, combined_stats);

      const RealVector& moments = approx_i.moments();  // virtual
      mu = moments[0];
      var = moments[1];  // Pecos provides central moments

      if (covarianceControl == DIAGONAL_COVARIANCE)
        respVariance[i] = var;
      else if (covarianceControl == FULL_COVARIANCE)
        respCovariance(i, i) = var;

      if (var >= 0.)
        sigma = std::sqrt(var);
      else {  // negative variance can happen with SC on sparse grids
        Cerr << "Warning: stochastic expansion variance is negative in "
             << "computation of std deviation.\n         Setting std "
             << "deviation to zero." << std::endl;
        sigma = 0.;
      }
    }

    // compute moment gradients if needed for beta mappings
    bool moment_grad_mapping_flag = false;
    if (respLevelTarget == RELIABILITIES) {
      total_offset = cntr + moment_offset;
      for (j = 0; j < rl_len;
           ++j)  // dbeta/ds requires mu, sigma, dmu/ds, dsigma/ds
        if (final_asv[total_offset + j] & 2) {
          moment_grad_mapping_flag = true;
          break;
        }
    }
    if (!moment_grad_mapping_flag) {
      total_offset = cntr + moment_offset + rl_len + pl_len;
      for (j = 0; j < bl_len; ++j)  // dz/ds requires dmu/ds, dsigma/ds
        if (final_asv[total_offset + j] & 2) {
          moment_grad_mapping_flag = true;
          break;
        }
    }

    bool final_mom1_flag = false, final_mom1_grad_flag = false;
    if (finalMomentsType) {
      final_mom1_flag = (final_asv[cntr] & 1);
      final_mom1_grad_flag = (final_asv[cntr] & 2);
    }
    bool mom1_grad_flag = (final_mom1_grad_flag || moment_grad_mapping_flag);
    // *** mean (Note: computation above not based on final ASV)
    if (final_mom1_flag) finalStatistics.function_value(mu, cntr);
    // *** mean gradient
    if (mom1_grad_flag) {
      const RealVector& grad =
          (allVars) ? approx_i.mean_gradient(initialPtU, final_dvv)
                    : approx_i.mean_gradient();
      if (final_mom1_grad_flag) finalStatistics.function_gradient(grad, cntr);
      if (moment_grad_mapping_flag) mu_grad = grad;  // transfer to code below
    }
    if (finalMomentsType) ++cntr;

    bool final_mom2_flag = false, final_mom2_grad_flag = false;
    if (finalMomentsType) {
      final_mom2_flag = (final_asv[cntr] & 1);
      final_mom2_grad_flag = (final_asv[cntr] & 2);
    }
    bool mom2_grad_flag = (final_mom2_grad_flag || moment_grad_mapping_flag);
    bool std_moments = (finalMomentsType == Pecos::STANDARD_MOMENTS);
    // *** std dev / variance (Note: computation above not based on final ASV)
    if (final_mom2_flag) {
      if (std_moments)
        finalStatistics.function_value(sigma, cntr);
      else
        finalStatistics.function_value(var, cntr);
    }
    // *** std deviation / variance gradient
    if (mom2_grad_flag) {
      const RealVector& grad =
          (allVars) ? approx_i.variance_gradient(initialPtU, final_dvv)
                    : approx_i.variance_gradient();
      if (std_moments || moment_grad_mapping_flag) {
        if (sigma_grad.empty())
          sigma_grad.sizeUninitialized(num_final_grad_vars);
        if (sigma > 0.)
          for (j = 0; j < num_final_grad_vars; ++j)
            sigma_grad[j] = grad[j] / (2. * sigma);
        else {
          Cerr << "Warning: stochastic expansion std deviation is zero in "
               << "computation of std deviation gradient.\n         Setting "
               << "gradient to zero." << std::endl;
          sigma_grad = 0.;
        }
      }
      if (final_mom2_grad_flag) {
        if (std_moments)
          finalStatistics.function_gradient(sigma_grad, cntr);
        else
          finalStatistics.function_gradient(grad, cntr);
      }
    }
    if (finalMomentsType) ++cntr;

    if (respLevelTarget == RELIABILITIES) {
      for (j = 0; j < rl_len; ++j, ++cntr) {
        // *** beta
        if (final_asv[cntr] & 1) {
          Real z_bar = requestedRespLevels[i][j];
          if (!Pecos::is_small(sigma)) {
            Real ratio = (mu - z_bar) / sigma;
            computedRelLevels[i][j] = beta = (cdfFlag) ? ratio : -ratio;
          } else
            computedRelLevels[i][j] = beta =
                ((cdfFlag && mu <= z_bar) || (!cdfFlag && mu > z_bar))
                    ? -Pecos::LARGE_NUMBER
                    : Pecos::LARGE_NUMBER;
          finalStatistics.function_value(beta, cntr);
        }
        // *** beta gradient
        if (final_asv[cntr] & 2) {
          if (final_stat_grad.empty())
            final_stat_grad.sizeUninitialized(num_final_grad_vars);
          if (!Pecos::is_small(sigma)) {
            Real z_bar = requestedRespLevels[i][j];
            for (k = 0; k < num_final_grad_vars; ++k) {
              Real dratio_dx =
                  (sigma * mu_grad[k] - (mu - z_bar) * sigma_grad[k]) /
                  std::pow(sigma, 2);
              final_stat_grad[k] = (cdfFlag) ? dratio_dx : -dratio_dx;
            }
          } else
            final_stat_grad = 0.;
          finalStatistics.function_gradient(final_stat_grad, cntr);
        }
      }
    } else
      cntr += rl_len;

    // no analytic mappings for probability levels
    cntr += pl_len;

    for (j = 0; j < bl_len; ++j, ++cntr) {
      Real beta_bar = requestedRelLevels[i][j];
      if (final_asv[cntr] & 1) {
        // *** z
        computedRespLevels[i][j + pl_len] = z =
            (cdfFlag) ? mu - beta_bar * sigma : mu + beta_bar * sigma;
        finalStatistics.function_value(z, cntr);
      }
      if (final_asv[cntr] & 2) {
        // *** z gradient
        if (final_stat_grad.empty())
          final_stat_grad.sizeUninitialized(num_final_grad_vars);
        for (k = 0; k < num_final_grad_vars; ++k)
          final_stat_grad[k] = (cdfFlag)
                                   ? mu_grad[k] - beta_bar * sigma_grad[k]
                                   : mu_grad[k] + beta_bar * sigma_grad[k];
        finalStatistics.function_gradient(final_stat_grad, cntr);
      }
    }

    // no analytic mappings for generalized reliability levels
    cntr += gl_len;

    // *** local sensitivities
    if (local_grad_stats && approx_i.expansion_coefficient_flag()) {
      // expansion sensitivities are defined from the coefficients and basis
      // polynomial derivatives.  They are computed for the means of the
      // uncertain varables and provide a measure of local importance (but not
      // scaled by input covariance as in mean value importance factors).
      Pecos::MultivariateDistribution& x_dist =
          iteratedModel->multivariate_distribution();
      const RealVector& exp_grad_u =
          approx_i.gradient(uSpaceModel->current_variables());
      RealVector exp_grad_x(
          Teuchos::getCol(Teuchos::View, expGradsMeanX, (int)i));
      uSpaceModel->trans_grad_U_to_X(exp_grad_u, exp_grad_x, x_dist.means());

#ifdef TEST_HESSIANS
      const RealSymMatrix& exp_hess_u =
          approx_i.hessian(uSpaceModel->current_variables());
      // RealSymMatrix exp_hess_x;
      // uSpaceModel->trans_hess_U_to_X(exp_hess_u, exp_hess_x, x_dist.means());
      Cout << exp_hess_u;  //<< exp_hess_x;
#endif                     // TEST_HESSIANS
    }

    // *** global sensitivities:
    if (vbdFlag && approx_i.expansion_coefficient_flag()) {
      approx_i.compute_component_effects();  // main or main+interaction
      approx_i.compute_total_effects();      // total
    }
  }

  if (covarianceControl == FULL_COVARIANCE)
    compute_off_diagonal_covariance();  // diagonal entries were filled in above
}

void NonDExpansion::compute_numerical_statistics() {
  // for use with FINAL_RESULTS state

  if (!expansionSampler) return;

  RealVector exp_sampler_stats;
  RealVectorArray imp_sampler_stats;
  RealRealPairArray min_max_fns;
  ShortArray sampler_asv;
  define_sampler_asv(sampler_asv);
  run_sampler(sampler_asv, exp_sampler_stats);
  refine_sampler(imp_sampler_stats, min_max_fns);

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  bool list_sampling = (expansionSampler->method_name() == LIST_SAMPLING),
       imp_sampling = bool(importanceSampler);
  std::shared_ptr<NonDSampling> exp_sampler_rep =
      std::static_pointer_cast<NonDSampling>(expansionSampler);
  size_t i, j,
      cntr = 0, sampler_cntr = 0, moment_offset = (finalMomentsType) ? 2 : 0,
      sampler_moment_offset = (exp_sampler_rep->final_moments_type()) ? 2 : 0;
  Real p, gen_beta, z;

  // Update finalStatistics from {exp,imp}_sampler_stats.  Moment mappings
  // are not recomputed since empty level arrays are passed in construct_
  // expansion_sampler() and these levels are omitted from sampler_cntr.
  archive_allocate_mappings();

  for (i = 0; i < numFunctions; ++i) {
    cntr += moment_offset;
    // sampler_cntr tracks only the numerical stats (analytic level mappings
    // and final moments are suppressed in construct_expansion_sampler())
    sampler_cntr += sampler_moment_offset;

    if (respLevelTarget == RELIABILITIES)
      cntr += requestedRespLevels[i].length();  // don't increment sampler_cntr
    else {
      size_t rl_len = requestedRespLevels[i].length();
      for (j = 0; j < rl_len; ++j, ++cntr, ++sampler_cntr) {
        if (final_asv[cntr] & 1) {
          p = (imp_sampling) ? imp_sampler_stats[i][j]
                             : exp_sampler_stats[sampler_cntr];
          if (respLevelTarget == PROBABILITIES) {
            computedProbLevels[i][j] = p;
            finalStatistics.function_value(p, cntr);
          } else if (respLevelTarget == GEN_RELIABILITIES) {
            computedGenRelLevels[i][j] = gen_beta =
                -Pecos::NormalRandomVariable::inverse_std_cdf(p);
            finalStatistics.function_value(gen_beta, cntr);
          }
        }
        if (final_asv[cntr] & 2) {  // TO DO: sampling sensitivity analysis
          Cerr << "\nError: analytic sensitivity of response ";
          if (respLevelTarget == PROBABILITIES)
            Cerr << "probability";
          else if (respLevelTarget == GEN_RELIABILITIES)
            Cerr << "generalized reliability";
          Cerr << " not yet supported." << std::endl;
          abort_handler(METHOD_ERROR);
        }
      }
    }

    size_t pl_len = requestedProbLevels[i].length();
    for (j = 0; j < pl_len; ++j, ++cntr, ++sampler_cntr) {
      if (final_asv[cntr] & 1) {
        computedRespLevels[i][j] = z = exp_sampler_stats[sampler_cntr];
        finalStatistics.function_value(z, cntr);
      }
      if (final_asv[cntr] & 2) {  // TO DO: p->z sampling sensitivity analysis
        Cerr << "\nError: analytic sensitivity of response level not yet "
             << "supported for mapping from probability." << std::endl;
        abort_handler(METHOD_ERROR);
      }
    }

    size_t bl_len = requestedRelLevels[i].length();
    cntr += bl_len;  // don't increment sampler_cntr

    size_t gl_len = requestedGenRelLevels[i].length();
    for (j = 0; j < gl_len; ++j, ++cntr, ++sampler_cntr) {
      if (final_asv[cntr] & 1) {
        computedRespLevels[i][j + pl_len + bl_len] = z =
            exp_sampler_stats[sampler_cntr];
        finalStatistics.function_value(z, cntr);
      }
      if (final_asv[cntr] & 2) {  // TO DO: beta*->p->z sampling SA
        Cerr << "\nError: analytic sensitivity of response level not yet "
             << "supported for mapping from generalized reliability."
             << std::endl;
        abort_handler(METHOD_ERROR);
      }
    }

    // archive the mappings from/to response levels
    archive_from_resp(i);
    archive_to_resp(i);
  }

  // now that level arrays are updated, infer the PDFs.
  // imp_sampling flag prevents mixing of refined and unrefined level mappings.
  compute_densities(min_max_fns, imp_sampling);
  for (i = 0; i < numFunctions; ++i) archive_pdf(i);
}

void NonDExpansion::compute_numerical_stat_refinements(
    RealVectorArray& imp_sampler_stats, RealRealPairArray& min_max_fns) {
  // response fn is active for z->p, z->beta*, p->z, or beta*->z

  const RealMatrix& exp_vars = expansionSampler->all_samples();
  // const IntResponseMap& exp_responses = expansionSampler->all_responses();
  const RealVector& exp_sampler_stats =
      expansionSampler->response_results().function_values();
  int exp_cv = exp_vars.numRows();

  std::shared_ptr<NonDSampling> exp_sampler_rep =
      std::static_pointer_cast<NonDSampling>(expansionSampler);
  std::shared_ptr<NonDAdaptImpSampling> imp_sampler_rep =
      std::static_pointer_cast<NonDAdaptImpSampling>(importanceSampler);

  size_t i, j, exp_sampler_cntr = 0;
  imp_sampler_stats.resize(numFunctions);
  bool x_data_flag = false;
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  size_t exp_sampler_moment_offset =
      (exp_sampler_rep->final_moments_type()) ? 2 : 0;
  for (i = 0; i < numFunctions; ++i) {
    // exp_sampler_cntr tracks only the numerical stats (analytic level mappings
    // and final moments are suppressed in construct_expansion_sampler())
    exp_sampler_cntr += exp_sampler_moment_offset;
    size_t rl_len = requestedRespLevels[i].length();
    if (rl_len && respLevelTarget != RELIABILITIES) {
      imp_sampler_stats[i].resize(rl_len);
      // initializing importance sampling with both original build
      // points and LHS expansion sampler points (var only, no resp)
      const Pecos::SurrogateData& exp_data = uSpaceModel->approximation_data(i);
      size_t m, num_data_pts = exp_data.points(),
                num_to_is = numSamplesOnExpansion + num_data_pts;
      RealVectorArray initial_points(num_to_is);
      const Pecos::SDVArray& sdv_array = exp_data.variables_data();
      for (m = 0; m < num_data_pts; ++m)
        initial_points[m] = sdv_array[m].continuous_variables();  // view OK
      for (m = 0; m < numSamplesOnExpansion; ++m)
        copy_data(exp_vars[m], exp_cv, initial_points[m + num_data_pts]);
      for (j = 0; j < rl_len; ++j, ++exp_sampler_cntr) {
        // Cout << "Initial estimate of p to seed "
        //      << exp_sampler_stats[exp_sampler_cntr] << '\n';
        imp_sampler_rep->initialize(initial_points, x_data_flag, i,
                                    exp_sampler_stats[exp_sampler_cntr],
                                    requestedRespLevels[i][j]);

        importanceSampler->run(pl_iter);

        // Real p = imp_sampler_rep->final_probability();
        // Cout << "importance sampling estimate for function " << i
        //      << " level " << j << " = " << p << "\n";
        imp_sampler_stats[i][j] = imp_sampler_rep->final_probability();
      }
    }
    // exp_sampler_cntr offset by moments, rl_len for p, pl_len, and gl_len
    exp_sampler_cntr +=
        requestedProbLevels[i].length() + requestedGenRelLevels[i].length();
  }
  // update min_max_fns for use in defining bounds for outer PDF bins
  if (pdfOutput) min_max_fns = imp_sampler_rep->extreme_values();
}

void NonDExpansion::pull_reference(RealVector& stats_ref) {
  if (!refineMetric) {
    Cerr << "Error: refineMetric definition required in NonDExpansion::"
         << "pull_reference()" << std::endl;
    abort_handler(METHOD_ERROR);
  }

  size_t mom_len = 0, lev_len = 0;
  bool full_covar = (covarianceControl == FULL_COVARIANCE);
  if (refineMetric == Pecos::COVARIANCE_METRIC ||
      refineMetric == Pecos::MIXED_STATS_METRIC)
    mom_len = (full_covar) ? (numFunctions * (numFunctions + 3)) / 2
                           : 2 * numFunctions;
  if (refineMetric == Pecos::LEVEL_STATS_METRIC ||
      refineMetric == Pecos::MIXED_STATS_METRIC)
    lev_len = totalLevelRequests;
  size_t stats_len = mom_len + lev_len;
  if (stats_ref.length() != stats_len) stats_ref.sizeUninitialized(stats_len);

  switch (refineMetric) {
    case Pecos::COVARIANCE_METRIC:
    case Pecos::MIXED_STATS_METRIC: {
      // pull means
      std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
      if (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS)
        for (size_t i = 0; i < numFunctions; ++i)
          stats_ref[i] = poly_approxs[i].combined_moment(0);
      else
        for (size_t i = 0; i < numFunctions; ++i)
          stats_ref[i] = poly_approxs[i].moment(0);

      // pull resp{V,Cov}ariance (comb stats managed in compute_*_covariance())
      if (full_covar)
        pull_lower_triangle(respCovariance, stats_ref, numFunctions);
      else
        copy_data_partial(respVariance, stats_ref, numFunctions);
      break;
    }
  }

  switch (refineMetric) {
    case Pecos::LEVEL_STATS_METRIC:
    case Pecos::MIXED_STATS_METRIC:
      pull_level_mappings(stats_ref, mom_len);
      break;
  }

#ifdef DEBUG
  Cout << "Pulled stats:\n" << stats_ref;
#endif  // DEBUG
}

void NonDExpansion::push_reference(const RealVector& stats_ref) {
  if (!refineMetric) {
    Cerr << "Error: refineMetric definition required in NonDExpansion::"
         << "push_reference()" << std::endl;
    abort_handler(METHOD_ERROR);
  }

  bool full_covar = (covarianceControl == FULL_COVARIANCE);
  switch (refineMetric) {
    case Pecos::COVARIANCE_METRIC:
    case Pecos::MIXED_STATS_METRIC: {
      // push resp{V|Cov}ariance (extract first since reused below)
      if (full_covar)
        push_lower_triangle(stats_ref, respCovariance, numFunctions);
      else
        copy_data_partial(stats_ref, numFunctions, numFunctions, respVariance);

      // push Pecos::{expansion|numerical}Moments
      std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
      if (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS)
        for (size_t i = 0; i < numFunctions; ++i) {
          poly_approxs[i].combined_moment(stats_ref[i], 0);  // mean values
          if (full_covar)
            poly_approxs[i].combined_moment(respCovariance(i, i), 1);
          else
            poly_approxs[i].combined_moment(respVariance[i], 1);
        }
      else
        for (size_t i = 0; i < numFunctions; ++i) {
          poly_approxs[i].moment(stats_ref[i], 0);  // mean values
          if (full_covar)
            poly_approxs[i].moment(respCovariance(i, i), 1);
          else
            poly_approxs[i].moment(respVariance[i], 1);
        }
      break;
    }
  }

  switch (refineMetric) {
    case Pecos::LEVEL_STATS_METRIC:
      push_level_mappings(stats_ref, 0);
      break;
    case Pecos::MIXED_STATS_METRIC: {
      size_t offset = (full_covar) ? (numFunctions * (numFunctions + 3)) / 2
                                   : 2 * numFunctions;
      push_level_mappings(stats_ref, offset);
      break;
    }
  }

#ifdef DEBUG
  Cout << "Pushed stats:\n" << stats_ref;
#endif  // DEBUG
}

void NonDExpansion::define_sampler_asv(ShortArray& sampler_asv) {
  if (expansionSampler && expansionSampler->method_name() == LIST_SAMPLING)
    sampler_asv.assign(numFunctions, 1);
  else {
    sampler_asv.assign(numFunctions, 0);
    // response fn is active for z->p, z->beta*, p->z, or beta*->z
    const ShortArray& final_asv = finalStatistics.active_set_request_vector();
    size_t i, j, cntr = 0, moment_offset = (finalMomentsType) ? 2 : 0;
    for (i = 0; i < numFunctions; ++i) {
      cntr += moment_offset;
      size_t rl_len = requestedRespLevels[i].length();
      if (respLevelTarget != RELIABILITIES)
        for (j = 0; j < rl_len; ++j)
          if (final_asv[cntr + j] & 1) {
            sampler_asv[i] |= 1;
            break;
          }
      cntr += rl_len;
      size_t pl_len = requestedProbLevels[i].length();
      for (j = 0; j < pl_len; ++j)
        if (final_asv[cntr + j] & 1) {
          sampler_asv[i] |= 1;
          break;
        }
      cntr += pl_len + requestedRelLevels[i].length();
      size_t gl_len = requestedGenRelLevels[i].length();
      for (j = 0; j < gl_len; ++j)
        if (final_asv[cntr + j] & 1) {
          sampler_asv[i] |= 1;
          break;
        }
      cntr += gl_len;
    }
  }
}

void NonDExpansion::run_sampler(const ShortArray& sampler_asv,
                                RealVector& exp_sampler_stats) {
  if (!expansionSampler) return;

  expansionSampler->active_set_request_vector(sampler_asv);

  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  expansionSampler->run(pl_iter);

  std::shared_ptr<NonDSampling> exp_sampler_rep =
      std::static_pointer_cast<NonDSampling>(expansionSampler);
  if (expansionSampler->method_name() == LIST_SAMPLING)
    // full set of numerical statistics, including PDFs
    exp_sampler_rep->compute_statistics(expansionSampler->all_samples(),
                                        expansionSampler->all_responses());
  else {  // augment moment-based with sampling-based mappings (PDFs suppressed)
    exp_sampler_rep->compute_level_mappings(expansionSampler->all_responses());
    exp_sampler_rep->update_final_statistics();
  }
  exp_sampler_stats = expansionSampler->response_results().function_values();
}

void NonDExpansion::refine_sampler(RealVectorArray& imp_sampler_stats,
                                   RealRealPairArray& min_max_fns) {
  // Update probability estimates with importance sampling, if requested.
  if (importanceSampler)
    compute_numerical_stat_refinements(imp_sampler_stats, min_max_fns);
  else if (pdfOutput && expansionSampler) {
    // NonDSampling::extremeValues not avail (pdfOutput off)
    std::shared_ptr<NonDSampling> exp_sampler_rep =
        std::static_pointer_cast<NonDSampling>(expansionSampler);
    exp_sampler_rep->compute_intervals(min_max_fns);
  }
}

void NonDExpansion::archive_moments() {
  if (!resultsDB.active()) return;

  // for now, archive only central moments to avoid duplicating all above logic
  // insert NaN for missing data
  bool exp_active = false, num_active = false;
  RealMatrix exp_matrix(4, numFunctions), num_matrix(4, numFunctions);
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (size_t i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag()) {
      // Pecos provides central moments
      const RealVector& exp_moments = approx_i.expansion_moments();
      const RealVector& num_int_moments =
          approx_i.numerical_integration_moments();
      size_t exp_mom = exp_moments.length(),
             num_int_mom = num_int_moments.length();
      if (exp_mom) exp_active = true;
      if (num_int_mom) num_active = true;
      for (size_t j = 0; j < exp_mom; ++j) exp_matrix(j, i) = exp_moments[j];
      for (size_t j = exp_mom; j < 4; ++j)
        exp_matrix(j, i) = std::numeric_limits<Real>::quiet_NaN();
      for (size_t j = 0; j < num_int_mom; ++j)
        num_matrix(j, i) = num_int_moments[j];
      for (size_t j = num_int_mom; j < 4; ++j)
        num_matrix(j, i) = std::numeric_limits<Real>::quiet_NaN();
    }
  }

  // Set moments labels.
  std::string moment_1 = "Mean";
  std::string moment_2 = (finalMomentsType == Pecos::CENTRAL_MOMENTS)
                             ? "Variance"
                             : "Standard Deviation";
  std::string moment_3 =
      (finalMomentsType == Pecos::CENTRAL_MOMENTS) ? "3rd Central" : "Skewness";
  std::string moment_4 =
      (finalMomentsType == Pecos::CENTRAL_MOMENTS) ? "4th Central" : "Kurtosis";

  std::string moment_1_lower = "mean";
  std::string moment_2_lower = (finalMomentsType == Pecos::CENTRAL_MOMENTS)
                                   ? "variance"
                                   : "std_deviation";
  std::string moment_3_lower = (finalMomentsType == Pecos::CENTRAL_MOMENTS)
                                   ? "third_central"
                                   : "skewness";
  std::string moment_4_lower = (finalMomentsType == Pecos::CENTRAL_MOMENTS)
                                   ? "fourth_central"
                                   : "kurtosis";

  if (exp_active || num_active) {
    MetaDataType md_moments;
    md_moments["Row Labels"] =
        make_metadatavalue(moment_1, moment_2, moment_3, moment_4);
    md_moments["Column Labels"] =
        make_metadatavalue(ModelUtils::response_labels(*iteratedModel));

    if (exp_active) {
      resultsDB.insert(run_identifier(), resultsNames.moments_central_exp,
                       exp_matrix, md_moments);
      for (int i = 0; i < numFunctions; ++i) {
        DimScaleMap scales;
        scales.emplace(0, StringScale("moments",
                                      {moment_1_lower, moment_2_lower,
                                       moment_3_lower, moment_4_lower},
                                      ScaleScope::SHARED));
        RealVector moments;
        if (finalMomentsType == Pecos::CENTRAL_MOMENTS) {
          moments = poly_approxs[i].expansion_moments();
        } else {
          Pecos::PolynomialApproximation::standardize_moments(
              poly_approxs[i].expansion_moments(), moments);
        }
        resultsDB.insert(run_identifier(),
                         {String("expansion_moments"),
                          ModelUtils::response_labels(*iteratedModel)[i]},
                         moments, scales);
      }
    }
    if (num_active) {
      resultsDB.insert(run_identifier(), resultsNames.moments_central_num,
                       num_matrix, md_moments);
      for (int i = 0; i < numFunctions; ++i) {
        DimScaleMap scales;
        scales.emplace(0, StringScale("moments",
                                      {moment_1_lower, moment_2_lower,
                                       moment_3_lower, moment_4_lower},
                                      ScaleScope::SHARED));
        RealVector moments;
        if (finalMomentsType == Pecos::CENTRAL_MOMENTS) {
          moments = poly_approxs[i].numerical_integration_moments();
        } else {
          Pecos::PolynomialApproximation::standardize_moments(
              poly_approxs[i].numerical_integration_moments(), moments);
        }
        resultsDB.insert(run_identifier(),
                         {String("integration_moments"),
                          ModelUtils::response_labels(*iteratedModel)[i]},
                         moments, scales);
      }
    }
  }
}

void NonDExpansion::archive_sobol_indices() {
  // effects are computed per resp fn within compute_statistics()
  // if vbdFlag and expansion_coefficient_flag

  // this fn called if vbdFlag and prints per resp fn if
  // expansion_coefficient_flag and non-negligible variance
  if (!resultsDB.active()) return;

  const StringArray& fn_labels = ModelUtils::response_labels(*iteratedModel);
  StringMultiArrayConstView cv_labels =
      ModelUtils::continuous_variable_labels(*iteratedModel);

  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  // Map from index to variable labels
  std::map<int, std::vector<const char*> > sobol_labels;

  // Main, total, and each order of interactions are separately inserted
  // into resultsDB. orders maps the index of the Sobol' index to its order
  // to facilitate insertion.
  std::map<int, int> orders;

  // collect variable descriptors for interactions for (aggregated) sobol index
  // map. These will be used as dimension scales.
  size_t i, j, num_indices;
  if (vbdOrderLimit != 1) {  // unlimited (0) or includes interactions (>1)
    // create aggregate interaction labels (once for all response fns)
    std::shared_ptr<SharedApproxData> shared_data_rep =
        uSpaceModel->shared_approximation().data_rep();
    const Pecos::BitArrayULongMap& sobol_map =
        shared_data_rep->sobol_index_map();
    for (Pecos::BAULMCIter map_cit = sobol_map.begin();
         map_cit != sobol_map.end(); ++map_cit) {  // loop in key sorted order
      const BitArray& set = map_cit->first;
      unsigned long index = map_cit->second;  // 0-way -> n 1-way -> interaction
      if (index > numContinuousVars) {        // an interaction
        if (sobol_labels.find(index) == sobol_labels.end())
          sobol_labels[index] = std::vector<const char*>();
        orders[index] = set.count();
        for (j = 0; j < numContinuousVars; ++j) {
          if (set[j]) sobol_labels[index].push_back(cv_labels[j].c_str());
        }
      }
    }
  }

  // archive sobol indices per response function
  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag()) {
      // Note: vbdFlag can be defined for covarianceControl == NO_COVARIANCE.
      // In this case, we cannot screen effectively at this level.
      if (covarianceControl == DIAGONAL_COVARIANCE)
        assert(respVariance[i] >= 0.0);
      if (covarianceControl == FULL_COVARIANCE)
        assert(respCovariance(i, i) >= 0.0);
      bool well_posed =
          ((covarianceControl == DIAGONAL_COVARIANCE &&
            Pecos::is_small(std::sqrt(respVariance[i]), approx_i.mean())) ||
           (covarianceControl == FULL_COVARIANCE &&
            Pecos::is_small(std::sqrt(respCovariance(i, i)), approx_i.mean())))
              ? false
              : true;
      if (well_posed) {
        const RealVector& total_indices = approx_i.total_sobol_indices();
        const RealVector& sobol_indices = approx_i.sobol_indices();
        Pecos::ULongULongMap sparse_sobol_map =
            approx_i.sparse_sobol_index_map();
        bool dense = sparse_sobol_map.empty();
        Real sobol;
        size_t main_cntr = 0;
        // Store main effects and total effects
        RealArray main_effects, total_effects;
        StringArray scale_labels;
        for (j = 0; j < numContinuousVars; ++j) {
          if (dense)  // no compressive sensing
            sobol = sobol_indices[j + 1];
          else {  // for the compressive sensing case, storage is indirect
            Pecos::ULULMIter it = sparse_sobol_map.find(j + 1);
            if (it == sparse_sobol_map.end())
              sobol = 0.;
            else {
              sobol = sobol_indices[it->second];
              ++main_cntr;
            }
          }
          if (std::abs(sobol) > vbdDropTol ||
              std::abs(total_indices[j]) > vbdDropTol) {
            main_effects.push_back(sobol);
            total_effects.push_back(total_indices[j]);
            scale_labels.push_back(cv_labels[j]);
          }
        }
        if (!main_effects.empty()) {
          DimScaleMap scales;
          scales.emplace(
              0, StringScale("variables", scale_labels, ScaleScope::UNSHARED));
          resultsDB.insert(run_identifier(),
                           {String("main_effects"), fn_labels[i]}, main_effects,
                           scales);
          resultsDB.insert(run_identifier(),
                           {String("total_effects"), fn_labels[i]},
                           total_effects, scales);
        }

        // Print Interaction effects
        if (vbdOrderLimit !=
            1) {  // unlimited (0) or includes interactions (>1)
          RealArray int_effects;
          std::vector<std::vector<const char*> > int_scale;
          num_indices = sobol_indices.length();
          // Results file insertions are performed separately for each order.
          // old_order is used to detect when the order switches to trigger an
          // insertion.
          int old_order;
          if (dense) {  // no compressive sensing
            j = numContinuousVars + 1;
            // make sure index j exists before attempting to set old_order
            old_order = (j < num_indices) ? orders[j] : 0;
            for (; j < num_indices; ++j) {
              if (orders[j] != old_order && !int_effects.empty()) {
                String result_name = String("order_") +
                                     std::to_string(old_order) +
                                     String("_interactions");
                DimScaleMap int_scales;
                int_scales.emplace(0, StringScale("variables", int_scale,
                                                  ScaleScope::UNSHARED));
                resultsDB.insert(run_identifier(), {result_name, fn_labels[i]},
                                 int_effects, int_scales);
                int_scale.clear();
                int_effects.clear();
              }
              old_order = orders[j];
              if (std::abs(sobol_indices[j]) >
                  vbdDropTol) {  // print interaction
                int_effects.push_back(sobol_indices[j]);
                int_scale.push_back(sobol_labels[j]);
              }
            }
          } else {  // for the compressive sensing case, storage is indirect
            Pecos::ULULMIter it = ++sparse_sobol_map.begin();  // skip 0-way
            std::advance(it, main_cntr);  // advance past 1-way
            // make sure it->first exists before attempting to set old_order
            old_order = (it != sparse_sobol_map.end()) ? orders[it->first] : 0;
            for (; it != sparse_sobol_map.end(); ++it) {  // 2-way and above
              if (orders[it->first] != old_order && !int_effects.empty()) {
                String result_name = String("order_") +
                                     std::to_string(old_order) +
                                     String("_interactions");
                DimScaleMap int_scales;
                int_scales.emplace(0, StringScale("variables", int_scale,
                                                  ScaleScope::UNSHARED));
                resultsDB.insert(run_identifier(), {result_name, fn_labels[i]},
                                 int_effects, int_scales);
                int_scale.clear();
                int_effects.clear();
              }
              old_order = orders[it->first];
              sobol = sobol_indices[it->second];
              if (std::abs(sobol) > vbdDropTol) {  // print interaction
                int_effects.push_back(sobol);
                int_scale.push_back(sobol_labels[it->first]);
              }
            }
          }
          if (!int_effects.empty()) {  // Insert the remaining contents of
                                       // int_effects, if any
            String result_name = String("order_") + std::to_string(old_order) +
                                 String("_interactions");
            DimScaleMap int_scales;
            int_scales.emplace(
                0, StringScale("variables", int_scale, ScaleScope::UNSHARED));
            resultsDB.insert(run_identifier(), {result_name, fn_labels[i]},
                             int_effects, int_scales);
            int_scale.clear();
            int_effects.clear();
          }
        }
      }
    }
  }
}

void NonDExpansion::update_final_statistics_gradients() {
  if (finalStatistics.function_gradients().empty()) return;

  // Augmented design vars:
  // > All vars: transform dg/du to dg/dx -> provides desired dg/ds for x = s
  // > Distinct vars: PCE/SC approximations for dg/ds are formed
  // Inserted design vars:
  // > All and Distinct views for the subIterator are equivalent
  // > PCE/SC approximations for dg/ds are formed
  // > Alternative: All view could force an artificial cdv augmentation
  // Mixed augmented/inserted design vars:
  // > All vars: bookkeep the two dg/ds approaches
  // > Distinct vars: PCE/SC approximations for dg/ds are formed

  // for all_variables, finalStatistics design grads are in u-space
  // -> transform to the original design space
  if (allVars) {
    // this approach is more efficient but less general.  If we can assume
    // that the DVV only contains design/state vars, then we know they are
    // uncorrelated and the jacobian matrix is diagonal with terms 2./range.
    const SharedVariablesData& svd =
        iteratedModel->current_variables().shared_data();
    SizetMultiArrayConstView cv_ids =
        ModelUtils::continuous_variable_ids(*iteratedModel);
    const SizetArray& final_dvv =
        finalStatistics.active_set_derivative_vector();
    const std::vector<Pecos::RandomVariable>& x_ran_vars =
        iteratedModel->multivariate_distribution().random_variables();
    // Pecos::ProbabilityTransformation& nataf
    //   = uSpaceModel->probability_transformation();

    RealVector init_x;
    uSpaceModel->trans_U_to_X(initialPtU, init_x);
    RealMatrix final_stat_grads = finalStatistics.function_gradients_view();
    int num_final_stats = final_stat_grads.numCols();
    Real z, x, factor;
    size_t num_final_grad_vars = final_dvv.size(), i, j, rv_index, deriv_j,
           end_cauv = startCAUV + numCAUV;
    for (j = 0; j < num_final_grad_vars; ++j) {
      deriv_j = find_index(cv_ids, final_dvv[j]);  // final_dvv[j]-1;
      if (deriv_j < startCAUV || deriv_j >= end_cauv) {
        // design/epistemic/state: factor = 2/range (see jacobian_dZ_dX())
        rv_index = svd.cv_index_to_all_index(deriv_j);
        z = initialPtU[deriv_j];
        x = init_x[deriv_j];
        // nataf_rep->trans_Z_to_X(z, x, deriv_j); // private
        factor = x_ran_vars[rv_index].pdf(x) /
                 Pecos::UniformRandomVariable::std_pdf(z);
        for (i = 0; i < num_final_stats; ++i) final_stat_grads(j, i) *= factor;
      }
      // else inserted design variable sensitivity: no scaling required
    }

    // This approach is more general, but is overkill for this purpose
    // and incurs additional copying overhead.
    /*
    RealVector initial_pt_x_pv, fn_grad_u, fn_grad_x;
    copy_data(initial_pt_x, initial_pt_x_pv);
    RealMatrix jacobian_ux;
    nataf.jacobian_dU_dX(initial_pt_x_pv, jacobian_ux);
    RealBaseVector final_stat_grad;
    for (i=0; i<num_final_stats; ++i) {
      copy_data(finalStatistics.function_gradient_view(i), fn_grad_u);
      nataf.trans_grad_U_to_X(fn_grad_u, fn_grad_x, jacobian_ux, final_dvv);
      copy_data(fn_grad_x, final_stat_grad);
      finalStatistics.function_gradient(final_stat_grad, i)
    }
    */
  }

  // For distinct vars, nothing additional is needed since u_space_sampler
  // has been configured to compute dg/ds at each of the sample points (these
  // are already in user-space and are not part of the variable transform).
  // uSpaceModel->build_approximation() -> PecosApproximation::build()
  // then constructs PCE/SC approximations of these gradients, and
  // PecosApproximation::<mean,variance>_gradient()
  // are used above to generate dmu/ds, dsigma/ds, and dbeta/ds.
}

void NonDExpansion::print_moments(std::ostream& s) {
  s << std::scientific << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  const StringArray& fn_labels = ModelUtils::response_labels(*iteratedModel);
  size_t i, j, width = write_precision + 7;

  s << "\nMoment statistics for each response function:\n";

  // Handle cases of both expansion/numerical moments or only one or the other:
  //   both exp/num: SC and PCE with numerical integration
  //   exp only:     PCE with unstructured grids (regression, exp sampling)
  // Also handle numerical exception of negative variance in either exp or num
  size_t exp_mom, num_int_mom;
  bool exception = false, curr_exception, prev_exception = false,
       combined_stats = (statsMetricMode == Pecos::COMBINED_EXPANSION_STATS);
  RealVector std_exp_moments, std_num_int_moments, empty_moments;
  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (!approx_i.expansion_coefficient_flag()) continue;
    // Pecos provides central moments.  Note: combined moments could be
    // expansion or numerical integration, but what is important for header
    // output is that there is only one primary type with no secondary.
    const RealVector& exp_moments = (combined_stats)
                                        ? approx_i.combined_moments()
                                        : approx_i.expansion_moments();
    const RealVector& num_int_moments =
        (combined_stats) ? empty_moments
                         : approx_i.numerical_integration_moments();
    exp_mom = exp_moments.length();
    num_int_mom = num_int_moments.length();
    curr_exception = ((exp_mom == 2 && exp_moments[1] < 0.) ||
                      (num_int_mom == 2 && num_int_moments[1] < 0.) ||
                      (exp_mom > 2 && exp_moments[1] <= 0.) ||
                      (num_int_mom > 2 && num_int_moments[1] <= 0.));
    if (curr_exception || finalMomentsType == Pecos::CENTRAL_MOMENTS) {
      if (i == 0 || !prev_exception)
        s << std::setw(width + 15) << "Mean" << std::setw(width + 1)
          << "Variance" << std::setw(width + 1) << "3rdCentral"
          << std::setw(width + 2) << "4thCentral\n";
      if (exp_mom && num_int_mom)
        s << fn_labels[i];
      else
        s << std::setw(14) << fn_labels[i];
      if (exp_mom) {
        if (num_int_mom) s << '\n' << std::setw(14) << "expansion:  ";
        for (j = 0; j < exp_mom; ++j)
          s << ' ' << std::setw(width) << exp_moments[j];
      }
      if (num_int_mom) {
        if (exp_mom) s << '\n' << std::setw(14) << "integration:";
        for (j = 0; j < num_int_mom; ++j)
          s << ' ' << std::setw(width) << num_int_moments[j];
      }
      prev_exception = curr_exception;
      if (curr_exception && finalMomentsType == Pecos::STANDARD_MOMENTS)
        exception = true;
    } else {
      if (i == 0 || prev_exception)
        s << std::setw(width + 15) << "Mean" << std::setw(width + 1)
          << "Std Dev" << std::setw(width + 1) << "Skewness"
          << std::setw(width + 2) << "Kurtosis\n";
      if (exp_mom && num_int_mom)
        s << fn_labels[i];
      else
        s << std::setw(14) << fn_labels[i];
      if (exp_mom) {
        Pecos::PolynomialApproximation::standardize_moments(exp_moments,
                                                            std_exp_moments);
        if (num_int_mom) s << '\n' << std::setw(14) << "expansion:  ";
        for (j = 0; j < exp_mom; ++j)
          s << ' ' << std::setw(width) << std_exp_moments[j];
      }
      if (num_int_mom) {
        Pecos::PolynomialApproximation::standardize_moments(
            num_int_moments, std_num_int_moments);
        if (exp_mom) s << '\n' << std::setw(14) << "integration:";
        for (j = 0; j < num_int_mom; ++j)
          s << ' ' << std::setw(width) << std_num_int_moments[j];
      }
      prev_exception = false;
    }
    s << '\n';

    /* COV has been removed:
    if (std::abs(mean) > Pecos::SMALL_NUMBER)
      s << "  " << std::setw(width)   << std_dev/mean << '\n';
    else
      s << "  " << std::setw(width+1) << "Undefined\n";
    */
  }
  if (exception)
    s << "\nNote: due to non-positive variance (resulting from under-resolved "
      << "numerical integration),\n      standardized moments have been "
      << "replaced with central moments for at least one response.\n";
}

void NonDExpansion::print_covariance(std::ostream& s) {
  switch (covarianceControl) {
    case DIAGONAL_COVARIANCE:
      print_variance(s, respVariance);
      break;
    case FULL_COVARIANCE:
      print_covariance(s, respCovariance);
      break;
  }
}

void NonDExpansion::print_variance(std::ostream& s, const RealVector& resp_var,
                                   const String& prepend) {
  if (!resp_var.empty()) {
    if (prepend.empty())
      s << "\nVariance vector for response functions:\n";
    else
      s << '\n' << prepend << " variance vector for response functions:\n";
    write_col_vector_trans(s, 0, resp_var);
  }
}

void NonDExpansion::print_covariance(std::ostream& s,
                                     const RealSymMatrix& resp_covar,
                                     const String& prepend) {
  if (!resp_covar.empty()) {
    if (prepend.empty())
      s << "\nCovariance matrix for response functions:\n";
    else
      s << '\n' << prepend << " covariance matrix for response functions:\n";
    s << resp_covar;
  }
}

void NonDExpansion::print_sobol_indices(std::ostream& s) {
  // effects are computed per resp fn within compute_statistics()
  // if vbdFlag and expansion_coefficient_flag

  // this fn called if vbdFlag and prints per resp fn if
  // expansion_coefficient_flag and non-negligible variance

  s << "\nGlobal sensitivity indices for each response function:\n";

  const StringArray& fn_labels = ModelUtils::response_labels(*iteratedModel);
  StringMultiArrayConstView cv_labels =
      ModelUtils::continuous_variable_labels(*iteratedModel);

  // construct labels corresponding to (aggregated) sobol index map
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  StringArray sobol_labels;
  size_t i, j, num_indices;
  if (vbdOrderLimit != 1) {  // unlimited (0) or includes interactions (>1)
    // create aggregate interaction labels (once for all response fns)
    std::shared_ptr<SharedApproxData> shared_data_rep =
        uSpaceModel->shared_approximation().data_rep();
    const Pecos::BitArrayULongMap& sobol_map =
        shared_data_rep->sobol_index_map();
    sobol_labels.resize(sobol_map.size());
    for (Pecos::BAULMCIter map_cit = sobol_map.begin();
         map_cit != sobol_map.end(); ++map_cit) {  // loop in key sorted order
      const BitArray& set = map_cit->first;
      unsigned long index = map_cit->second;  // 0-way -> n 1-way -> interaction
      if (index > numContinuousVars) {        // an interaction
        String& label = sobol_labels[index];  // store in index order
        for (j = 0; j < numContinuousVars; ++j)
          if (set[j]) label += cv_labels[j] + " ";
      }
    }
  }

  // print sobol indices per response function
  for (i = 0; i < numFunctions; ++i) {
    Approximation& approx_i = poly_approxs[i];
    if (approx_i.expansion_coefficient_flag()) {
      // Skip output for negative (co-)variances
      // ---  Should more be done besides warning ? RWH
      if ((covarianceControl == DIAGONAL_COVARIANCE) &&
          (respVariance[i] < 0.0)) {
        s << fn_labels[i] << " Sobol' indices not available due to negative "
          << "variance\n";
        continue;
      }
      if ((covarianceControl == FULL_COVARIANCE) &&
          (respCovariance(i, i) < 0.0)) {
        s << fn_labels[i] << " Sobol' indices not available due to negative "
          << "covariance\n";
        continue;
      }

      // Note: vbdFlag can be defined for covarianceControl == NO_COVARIANCE.
      // In this case, we cannot screen effectively at this level.
      bool well_posed =
          ((covarianceControl == DIAGONAL_COVARIANCE &&
            Pecos::is_small(std::sqrt(respVariance[i]), approx_i.mean())) ||
           (covarianceControl == FULL_COVARIANCE &&
            Pecos::is_small(std::sqrt(respCovariance(i, i)), approx_i.mean())))
              ? false
              : true;
      if (well_posed) {
        const RealVector& total_indices = approx_i.total_sobol_indices();
        const RealVector& sobol_indices = approx_i.sobol_indices();
        Pecos::ULongULongMap sparse_sobol_map =
            approx_i.sparse_sobol_index_map();
        bool dense = sparse_sobol_map.empty();
        Real sobol;
        size_t main_cntr = 0;
        // Print Main and Total effects
        s << fn_labels[i] << " Sobol' indices:\n"
          << std::setw(38) << "Main" << std::setw(19) << "Total\n";
        for (j = 0; j < numContinuousVars; ++j) {
          if (dense)
            sobol = sobol_indices[j + 1];
          else {
            Pecos::ULULMIter it = sparse_sobol_map.find(j + 1);
            if (it == sparse_sobol_map.end())
              sobol = 0.;
            else {
              sobol = sobol_indices[it->second];
              ++main_cntr;
            }
          }
          if (std::abs(sobol) > vbdDropTol ||
              std::abs(total_indices[j]) > vbdDropTol)  // print main / total
            s << "                     " << std::setw(write_precision + 7)
              << sobol << ' ' << std::setw(write_precision + 7)
              << total_indices[j] << ' ' << cv_labels[j] << '\n';
        }
        // Print Interaction effects
        if (vbdOrderLimit !=
            1) {  // unlimited (0) or includes interactions (>1)
          num_indices = sobol_indices.length();
          s << std::setw(39) << "Interaction\n";
          if (dense) {
            for (j = numContinuousVars + 1; j < num_indices; ++j)
              if (std::abs(sobol_indices[j]) > vbdDropTol)  // print interaction
                s << "                     " << std::setw(write_precision + 7)
                  << sobol_indices[j] << ' ' << sobol_labels[j] << '\n';
          } else {
            Pecos::ULULMIter it = ++sparse_sobol_map.begin();  // skip 0-way
            std::advance(it, main_cntr);                  // advance past 1-way
            for (; it != sparse_sobol_map.end(); ++it) {  // 2-way and above
              sobol = sobol_indices[it->second];
              if (std::abs(sobol) > vbdDropTol)  // print interaction
                s << "                     " << std::setw(write_precision + 7)
                  << sobol << ' ' << sobol_labels[it->first] << '\n';
            }
          }
        }
      } else
        s << fn_labels[i] << " Sobol' indices not available due to negligible "
          << "variance\n";
    } else
      s << fn_labels[i] << " Sobol' indices not available due to expansion "
        << "coefficient mode\n";
  }
}

void NonDExpansion::print_local_sensitivity(std::ostream& s) {
  const StringArray& fn_labels = ModelUtils::response_labels(*iteratedModel);
  s << "\nLocal sensitivities for each response function evaluated at "
    << "uncertain variable means:\n";
  std::vector<Approximation>& poly_approxs = uSpaceModel->approximations();
  for (size_t i = 0; i < numFunctions; ++i)
    if (poly_approxs[i].expansion_coefficient_flag()) {
      s << fn_labels[i] << ":\n";
      write_col_vector_trans(s, (int)i, expGradsMeanX);
    }
}

void NonDExpansion::print_refinement_diagnostics(std::ostream& s) {
  // Output of the relevant refinement metrics occurs in
  // compute_{covariance,level_mapping,final_statistics}_metric();
  // additional refinement control diagnostics are output here:
  switch (refineControl) {
    case Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED:
      if (outputLevel >= DEBUG_OUTPUT) {
        // output fine-grained data on generalized index sets
        std::shared_ptr<NonDSparseGrid> nond_sparse =
            std::static_pointer_cast<NonDSparseGrid>(
                uSpaceModel->subordinate_iterator());
        nond_sparse->print_smolyak_multi_index();
      }
      break;
      /*
      // These calls induce redundant solves (also performed in NonDExpansion::
      // reduce_decay_rate_sets()), so, while these is no lag is in Sobol' case,
      // suppress per-QoI diagnostics for now to avoid redundant computation.
      case Pecos::DIMENSION_ADAPTIVE_CONTROL_DECAY:
        if (outputLevel >= NORMAL_OUTPUT) {
          // output spectral data for ML-MF UQ (for finer grain data, activate
          // DECAY_DEBUG in packages/pecos/src/OrthogPolyApproximation.cpp).
          std::vector<Approximation>& poly_approxs =
      uSpaceModel->approximations(); std::shared_ptr<PecosApproximation>
      poly_approx_rep; for (size_t i=0; i<numFunctions; ++i) { poly_approx_rep =
      std::static_pointer_cast<PecosApproximation>
              (poly_approxs[i].approx_rep());
            s << "Variable decay rates for response function " << i+1 << ":\n"
              << poly_approx_rep->dimension_decay_rates();
          }
        }
        break;
      // Sobol indices are lagging an iteration since computing these indices is
      // triggered by increment_grid_{preference,weights} on subsequent iter.
      // Better to output them (as derived quantities) from reduce_*() once
      // available downstream, rather than enforcing their compute/print as
      // standard Sobol' output upstream.
      case Pecos::DIMENSION_ADAPTIVE_CONTROL_SOBOL:
        if (outputLevel >= NORMAL_OUTPUT)
          print_sobol_indices(s); // from reduce_total_sobol_sets()
        break;
      */
  }
}

void NonDExpansion::print_results(std::ostream& s, short results_state) {
  switch (results_state) {
    case REFINEMENT_RESULTS: {
      // augment refinement output from compute_*_metric() [print_metric=true]
      if (outputLevel == DEBUG_OUTPUT) {
        // iteratedModel.print_evaluation_summary(s);
        switch (refineMetric) {
            // case Pecos::DEFAULT_METRIC: // not an option for refinement
          case Pecos::COVARIANCE_METRIC:
          case Pecos::MIXED_STATS_METRIC:
            // case Pecos::LEVEL_STATS_METRIC: // moments only computed if beta
            // mappings
            print_moments(s);
            break;
        }
      }

      print_refinement_diagnostics(s);
      break;
    }
    case INTERMEDIATE_RESULTS: {
      switch (refineMetric) {
        case Pecos::DEFAULT_METRIC:
          print_moments(s);
          if (totalLevelRequests) print_level_mappings(s);
          break;
        case Pecos::COVARIANCE_METRIC:
          print_moments(s);
          print_covariance(s);
          break;
        case Pecos::MIXED_STATS_METRIC:
          print_moments(s);
          print_level_mappings(s);
          break;
        case Pecos::LEVEL_STATS_METRIC:
          print_level_mappings(s);
          break;
      }

      // print_refinement_diagnostics(s);
      break;
    }
    case FINAL_RESULTS: {
      s << "-------------------------------------------------------------------"
           "--"
        << "--------\nStatistics derived analytically from polynomial "
           "expansion:"
        << '\n';
      print_moments(s);
      print_covariance(s);
      if (!subIteratorFlag && outputLevel >= NORMAL_OUTPUT)
        print_local_sensitivity(s);
      if (vbdFlag) print_sobol_indices(s);

      // Print level mapping statistics (typically from sampling on expansion)
      std::shared_ptr<NonDSampling> exp_sampler_rep =
          std::static_pointer_cast<NonDSampling>(expansionSampler);
      bool exp_sampling = (exp_sampler_rep != NULL),
           list_sampling = (exp_sampling &&
                            exp_sampler_rep->method_name() == LIST_SAMPLING);
      if (list_sampling) {
        // all stats delegated since local moments are not relevant in this case
        s << "-----------------------------------------------------------------"
             "--"
          << "----------\nStatistics based on " << numSamplesOnExpansion
          << " imported samples performed on polynomial expansion:\n";
        exp_sampler_rep->print_statistics(s);
      } else if (totalLevelRequests) {
        s << "-----------------------------------------------------------------"
             "--"
          << "----------\nStatistics based on ";
        if (exp_sampling)
          s << numSamplesOnExpansion << " samples performed on polynomial "
            << "expansion:\n";
        else
          s << "projection of analytic moments:\n";
        // no stats are delegated since we mix numerical stats with local
        // analytic moment-based projections (more accurate than sample
        // moment-based projections).  In addition, we may have local
        // probability refinements.
        print_level_mappings(s);
        print_system_mappings(
            s);  // works off of finalStatistics -> no delegation
      }
      s << "-------------------------------------------------------------------"
           "--"
        << "--------" << std::endl;
      break;
    }
  }
}

}  // namespace Dakota
