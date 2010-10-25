/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDExpansion
//- Description: Implementation code for NonDExpansion class
//- Owner:       Mike Eldred

#include "system_defs.h"
#include "data_io.h"
#include "NonDExpansion.H"
#include "NonDCubature.H"
#include "NonDQuadrature.H"
#include "NonDSparseGrid.H"
#include "NonDLHSSampling.H"
#include "NonDAdaptImpSampling.H" 
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "RecastModel.H"
#include "PecosApproximation.H"
#include "pecos_stat_util.hpp"
#include "SensAnalysisGlobal.H"

//#define DEBUG
//#define CONVERGENCE_DATA


namespace Dakota {

NonDExpansion::NonDExpansion(Model& model): NonD(model),
  expansionCoeffsApproach(-1), numUncertainQuant(0), numSamplesOnModel(0),
  numSamplesOnExpansion(probDescDB.get_int("method.samples")),
  useDerivsFlag(probDescDB.get_bool("method.nond.derivative_usage")),
  ruleNestingOverride(probDescDB.get_short("method.nond.nesting_override")),
  stochExpRefineType(
    probDescDB.get_short("method.nond.expansion_refinement_type")),
  stochExpRefineControl(
    probDescDB.get_short("method.nond.expansion_refinement_control")),
  betaMappings(false), expSampling(false)
{
  // Re-assign defaults specialized to refinement of stochastic expansions
  if (maxIterations < 0) // ctor chain picks up DataMethod default of -1
    maxIterations = 100;

  bool err_flag = false;
  if (useDerivsFlag && gradientType == "none") { // && hessianType == "none"
    Cerr << "\nError: use_derivatives option in NonDExpansion requires a "
	 << "response gradient specification." << std::endl;
    err_flag = true;
  }

  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cerr << "\nError: discrete random variables are not currently supported in "
	 << "NonDExpansion." << std::endl;
    err_flag = true;
  }

  if (probDescDB.get_bool("method.variance_based_decomp")) {
    vbdControl = probDescDB.get_short("method.nond.vbd_control");
    vbdDropTol = probDescDB.get_real("method.nond.vbd_drop_tolerance");
  }
  else
    vbdControl = Pecos::NO_VBD;
  // Note: minimum VBD control for variance-controlled refinement is
  //       enforced in NonDExpansion::construct_{quadrature,sparse_grid}
  //Cout << "VBD control = " << vbdControl << std::endl;

  natafTransform = Pecos::ProbabilityTransformation("nataf");
  const Pecos::RealSymMatrix& uncertain_corr
    = model.distribution_parameters().uncertain_correlations();
  if (!uncertain_corr.empty()) {
    natafTransform.initialize_random_variable_correlations(uncertain_corr);
    if (numContDesVars || numContEpistUncVars || numContStateVars)
      // expand ProbabilityTransformation::corrMatrixX to include design + state
      // + epistemic uncertain vars.  TO DO: propagate through model recursion?
      natafTransform.reshape_correlation_matrix(numContDesVars,
	numContAleatUncVars, numContEpistUncVars+numContStateVars);
  }

  // initialize meanStats, stdDevStats, and finalStatistics
  meanStats.size(numFunctions);
  stdDevStats.size(numFunctions);
  initialize_final_statistics();

  // -----------------------------------------------------------------
  // Determine full polynomial family, Wiener-Askey, or Wiener-Hermite
  // -----------------------------------------------------------------
  // use Askey/extended u-space definitions in Nataf transformations
  short u_space_type = probDescDB.get_short("method.nond.expansion_type");
  initialize_random_variable_types(u_space_type); // need ranVarTypesX/U below

  // check for correlations that are not supported for (1) use in extended
  // u-space and (2) use by Der Kiureghian & Liu for basic u-space (bounded
  // normal, bounded lognormal, loguniform, triangular, and beta vars).
  size_t i, j;
  if (natafTransform.x_correlation()) {

    const Pecos::ShortArray&    x_types = natafTransform.x_types();
    Pecos::ShortArray           u_types = natafTransform.u_types(); // copy
    const Pecos::RealSymMatrix& x_corr  = natafTransform.x_correlation_matrix();

    // We can only decorrelate in std normal space; therefore, if a variable
    // with a u_type other than STD_NORMAL is correlated with anything, revert
    // its u_type to STD_NORMAL.   Note: loop below must check all columns,
    // despite symmetry, since only rows are checked for u_type.
    bool u_space_modified = false;
    for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i)
      if (u_types[i] != Pecos::STD_NORMAL)
	for (j=numContDesVars; j<numContDesVars+numContAleatUncVars; ++j)
	  if (i != j && std::fabs(x_corr(i, j)) > 1.e-25) {
	    Cerr << "\nWarning: in NonDExpansion, u-space type for random "
		 << "variable " << i-numContDesVars+1 << " changed to\n"
		 << "         STD_NORMAL due to decorrelation requirements.\n";
	    u_types[i] = Pecos::STD_NORMAL; u_space_modified = true; break;
	  }

    if (u_space_modified) // update ranVarTypesU in natafTransform
      natafTransform.initialize_random_variable_types(x_types, u_types);

    // All correlated variables must have u_type = STD_NORMAL, but correlation
    // warping for bounded normal, bounded lognormal, loguniform, triangular,
    // beta, and histogram distributions are not supported by Der Kiureghian
    // & Liu.  Abort with an error in these cases.
    bool distribution_error = false;
    for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i)
      if ( x_types[i] == Pecos::BOUNDED_NORMAL ||
	   x_types[i] == Pecos::BOUNDED_LOGNORMAL ||
	   x_types[i] == Pecos::LOGUNIFORM || x_types[i] == Pecos::TRIANGULAR ||
	   x_types[i] == Pecos::BETA || x_types[i] == Pecos::HISTOGRAM_BIN )
	for (j=numContDesVars; j<numContDesVars+numContAleatUncVars; ++j)
	  if (i != j && std::fabs(x_corr(i, j)) > 1.e-25)
	    { distribution_error = true; break; }
    if (distribution_error) {
      Cerr << "\nError: correlation warping for Nataf variable transformation "
	   << "of bounded normal,\n       bounded lognormal, loguniform, "
	   << "triangular, beta, and histogram bin\n       distributions is "
	   << "not currently supported in NonDExpansion." << std::endl;
      err_flag = true;
    }
  }
  
  impSampling = false;
  if (err_flag)
    abort_handler(-1);
}


NonDExpansion::~NonDExpansion()
{ 
  if (impSampling)
    uSpaceModel.free_communicators(importanceSampler.maximum_concurrency());
}


void NonDExpansion::construct_g_u_model(Model& g_u_model)
{
  size_t i;
  Sizet2DArray vars_map, primary_resp_map, secondary_resp_map;
  vars_map.resize(numContinuousVars);
  for (i=0; i<numContinuousVars; ++i) {
    vars_map[i].resize(1);
    vars_map[i][0] = i;
  }
  primary_resp_map.resize(numFunctions);
  for (i=0; i<numFunctions; ++i) {
    primary_resp_map[i].resize(1);
    primary_resp_map[i][0] = i;
  }
  // Nataf is a nonlinear tranformation for all variables except Normals.
  // Nonlinear mappings require special ASV logic for transforming Hessians.
  const Pecos::ShortArray& x_types = natafTransform.x_types();
  const Pecos::ShortArray& u_types = natafTransform.u_types();
  bool nonlinear_vars_map = false;
  for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i)
    if ( x_types[i] != u_types[i] &&
	 !( x_types[i] == Pecos::NORMAL && u_types[i] == Pecos::STD_NORMAL ) &&
	 !( ( x_types[i] == Pecos::UNIFORM ||
	      x_types[i] == Pecos::HISTOGRAM_BIN ) &&
	    u_types[i] == Pecos::STD_UNIFORM ) &&
	 !( x_types[i] == Pecos::EXPONENTIAL &&
	    u_types[i] == Pecos::STD_EXPONENTIAL ) &&
	 !( x_types[i] == Pecos::BETA   && u_types[i] == Pecos::STD_BETA ) &&
	 !( x_types[i] == Pecos::GAMMA  && u_types[i] == Pecos::STD_GAMMA ) )
      { nonlinear_vars_map = true; break; }

  // There is no additional response mapping beyond that required by the
  // nonlinear variables mapping.
  BoolDequeArray nonlinear_resp_map(numFunctions, BoolDeque(1, false));
  g_u_model.assign_rep(new RecastModel(iteratedModel, vars_map,
    nonlinear_vars_map, vars_u_to_x_mapping, set_u_to_x_mapping,
    primary_resp_map, secondary_resp_map, 0, nonlinear_resp_map,
    resp_x_to_u_mapping, NULL), false);
  // Populate random variable distribution parameters for transformed u-space.
  // *** Note ***: For use with REGRESSION approaches, variable ordering in
  // get_parameter_sets() does not use x_types/u_types as in NonDQuadrature/
  // NonDSparseGrid and thus a possibility for future ordering errors exists.
  // Currently, Design/State -> Uniform is handled separately via global bounds
  // and no current x->u selection in NonD::initialize_random_variable_types()
  // causes an ordering discrepancy.  If a future mapping causes an ordering
  // inconsistency, this could be handled above via the RecastModel vars_map.
  size_t num_g_u_nuv = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::STD_NORMAL),
    num_g_u_bnuv     = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::BOUNDED_NORMAL),
    num_g_u_lnuv     = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::LOGNORMAL),
    num_g_u_blnuv    = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::BOUNDED_LOGNORMAL),
    num_g_u_uuv      =
      std::count(u_types.begin(), u_types.end(), (short)Pecos::STD_UNIFORM) - 
      std::count(x_types.begin(), x_types.end(), (short)Pecos::DESIGN) - 
      std::count(x_types.begin(), x_types.end(), (short)Pecos::STATE),
    num_g_u_luuv     = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::LOGUNIFORM),
    num_g_u_tuv      = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::TRIANGULAR),
    num_g_u_euv      = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::STD_EXPONENTIAL),
    num_g_u_buv      = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::STD_BETA),
    num_g_u_gauv     = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::STD_GAMMA),
    num_g_u_guuv     = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::GUMBEL),
    num_g_u_fuv      = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::FRECHET),
    num_g_u_wuv      = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::WEIBULL),
    num_g_u_hbuv     = std::count(u_types.begin(), u_types.end(),
				  (short)Pecos::HISTOGRAM_BIN);
  Pecos::DistributionParams& im_dp = iteratedModel.distribution_parameters();
  Pecos::DistributionParams& gu_dp = g_u_model.distribution_parameters();
  if (num_g_u_nuv || num_g_u_bnuv) {
    size_t num_total_nuv = num_g_u_nuv+num_g_u_bnuv;
    RealVector nuv_means(num_total_nuv, false),
      nuv_std_devs(num_total_nuv, false), nuv_l_bnds(num_total_nuv, false),
      nuv_u_bnds(num_total_nuv, false);
    if (num_g_u_bnuv) {
      const Pecos::RealVector& x_nuv_means    = im_dp.normal_means();
      const Pecos::RealVector& x_nuv_std_devs = im_dp.normal_std_deviations();
      const Pecos::RealVector& x_nuv_l_bnds   = im_dp.normal_lower_bounds();
      const Pecos::RealVector& x_nuv_u_bnds   = im_dp.normal_upper_bounds();
      size_t n_cntr = 0, x_n_cntr = 0;;
      for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i) {
	if (u_types[i] == Pecos::BOUNDED_NORMAL) {
	  nuv_means[n_cntr]    = x_nuv_means[x_n_cntr];
	  nuv_std_devs[n_cntr] = x_nuv_std_devs[x_n_cntr];
	  nuv_l_bnds[n_cntr]   = x_nuv_l_bnds[x_n_cntr];
	  nuv_u_bnds[n_cntr]   = x_nuv_u_bnds[x_n_cntr];
	  ++n_cntr;
	}
	else if (u_types[i] == Pecos::STD_NORMAL) {
	  nuv_means[n_cntr]  = 0.;	 nuv_std_devs[n_cntr] = 1.;
	  nuv_l_bnds[n_cntr] = -DBL_MAX; nuv_u_bnds[n_cntr]   = DBL_MAX;
	  ++n_cntr;
	}
	if (x_types[i] == Pecos::NORMAL || x_types[i] == Pecos::BOUNDED_NORMAL)
	  ++x_n_cntr;
      }
    }
    else {
      nuv_means  = 0.;       nuv_std_devs = 1.;
      nuv_l_bnds = -DBL_MAX; nuv_u_bnds   = DBL_MAX;
    }
    gu_dp.normal_means(nuv_means);
    gu_dp.normal_std_deviations(nuv_std_devs);
    gu_dp.normal_lower_bounds(nuv_l_bnds);
    gu_dp.normal_upper_bounds(nuv_u_bnds);
  }
  if (num_g_u_lnuv || num_g_u_blnuv) {
    gu_dp.lognormal_means(im_dp.lognormal_means());
    gu_dp.lognormal_std_deviations(im_dp.lognormal_std_deviations());
    gu_dp.lognormal_lambdas(im_dp.lognormal_lambdas());
    gu_dp.lognormal_zetas(im_dp.lognormal_zetas());
    gu_dp.lognormal_error_factors(im_dp.lognormal_error_factors());
    if (num_g_u_blnuv) {
      gu_dp.lognormal_lower_bounds(im_dp.lognormal_lower_bounds());
      gu_dp.lognormal_upper_bounds(im_dp.lognormal_upper_bounds());
    }
  }
  if (num_g_u_uuv) {
    RealVector uuv_l_bnds(num_g_u_uuv, false); uuv_l_bnds = -1.;
    RealVector uuv_u_bnds(num_g_u_uuv, false); uuv_u_bnds =  1.;
    gu_dp.uniform_lower_bounds(uuv_l_bnds);
    gu_dp.uniform_upper_bounds(uuv_u_bnds);
  }
  if (num_g_u_luuv) {
    gu_dp.loguniform_lower_bounds(im_dp.loguniform_lower_bounds());
    gu_dp.loguniform_upper_bounds(im_dp.loguniform_upper_bounds());
  }
  if (num_g_u_tuv) {
    gu_dp.triangular_modes(im_dp.triangular_modes());
    gu_dp.triangular_lower_bounds(im_dp.triangular_lower_bounds());
    gu_dp.triangular_upper_bounds(im_dp.triangular_upper_bounds());
  }
  if (num_g_u_euv) {
    RealVector euv_betas(num_g_u_euv, false); euv_betas = 1.;
    gu_dp.exponential_betas(euv_betas);
  }
  if (num_g_u_buv) {
    RealVector buv_l_bnds(num_g_u_buv, false); buv_l_bnds = -1.;
    RealVector buv_u_bnds(num_g_u_buv, false); buv_u_bnds =  1.;
    gu_dp.beta_alphas(im_dp.beta_alphas());
    gu_dp.beta_betas(im_dp.beta_betas());
    gu_dp.beta_lower_bounds(buv_l_bnds);
    gu_dp.beta_upper_bounds(buv_u_bnds);
  }
  if (num_g_u_gauv) {
    gu_dp.gamma_alphas(im_dp.gamma_alphas());
    RealVector gauv_betas(num_g_u_gauv, false); gauv_betas = 1.;
    gu_dp.gamma_betas(gauv_betas);
  }
  if (num_g_u_guuv) {
    gu_dp.gumbel_alphas(im_dp.gumbel_alphas());
    gu_dp.gumbel_betas(im_dp.gumbel_betas());
  }
  if (num_g_u_fuv) {
    gu_dp.frechet_alphas(im_dp.frechet_alphas());
    gu_dp.frechet_betas(im_dp.frechet_betas());
  }
  if (num_g_u_wuv) {
    gu_dp.weibull_alphas(im_dp.weibull_alphas());
    gu_dp.weibull_betas(im_dp.weibull_betas());
  }
  if (num_g_u_hbuv)
    gu_dp.histogram_bin_pairs(im_dp.histogram_bin_pairs());

  if (numContDesVars || numContEpistUncVars || numContStateVars) {
    // [-1,1] are standard bounds for design, state, epistemic, uniform, & beta
    RealVector c_l_bnds(numContinuousVars, false); c_l_bnds = -1.;
    RealVector c_u_bnds(numContinuousVars, false); c_u_bnds =  1.;
    // handle nonstandard bounds
    const RealVector& x_c_l_bnds = iteratedModel.continuous_lower_bounds();
    const RealVector& x_c_u_bnds = iteratedModel.continuous_upper_bounds();
    const RealVector& lnuv_means     = im_dp.lognormal_means();
    const RealVector& lnuv_std_devs  = im_dp.lognormal_std_deviations();
    const RealVector& lnuv_lambdas   = im_dp.lognormal_lambdas();
    const RealVector& lnuv_zetas     = im_dp.lognormal_zetas();
    const RealVector& lnuv_err_facts = im_dp.lognormal_error_factors();
    const RealVector& gauv_alphas = im_dp.gamma_alphas();
    const RealVector& guuv_alphas = im_dp.gumbel_alphas();
    const RealVector& guuv_betas  = im_dp.gumbel_betas();
    const RealVector& fuv_alphas  = im_dp.frechet_alphas();
    const RealVector& fuv_betas   = im_dp.frechet_betas();
    const RealVector& wuv_alphas  = im_dp.weibull_alphas();
    const RealVector& wuv_betas   = im_dp.weibull_betas();
    size_t i, lnuv_cntr = 0, gauv_cntr = 0, guuv_cntr = 0, fuv_cntr = 0,
      wuv_cntr = 0;
    for (i=numContDesVars; i<numContDesVars+numContAleatUncVars; ++i) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL:
	c_l_bnds[i] = -10.; c_u_bnds[i] = 10.; break; // mean +/- 10 std devs
      case Pecos::STD_EXPONENTIAL: // mean + 10 std devs for beta=1
	c_l_bnds[i] =   0.; c_u_bnds[i] = 11.; break;
      case Pecos::STD_GAMMA: {
	Real mean, stdev;
	Pecos::moments_from_gamma_params(gauv_alphas[guuv_cntr], 1.,
					 mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + 10.*stdev; ++gauv_cntr; break;
      }
      case Pecos::BOUNDED_NORMAL: case Pecos::BOUNDED_LOGNORMAL:
      case Pecos::LOGUNIFORM:     case Pecos::TRIANGULAR:
      case Pecos::HISTOGRAM_BIN:
	// bounded distributions: x-space has desired bounds
	c_l_bnds[i] = x_c_l_bnds[i]; c_u_bnds[i] = x_c_u_bnds[i]; break;
      // Note: Could use x_c_l_bnds/x_c_u_bnds for the following cases as well
      // except that NIDR uses +/-3 sigma, whereas here we're using +/-10 sigma
      case Pecos::LOGNORMAL: { // semi-bounded distribution
	Real mean, stdev;
	Pecos::moments_from_lognormal_spec(lnuv_means, lnuv_std_devs,
					   lnuv_lambdas, lnuv_zetas,
					   lnuv_err_facts, lnuv_cntr,
					   mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + 10.*stdev; ++lnuv_cntr; break;
      }
      case Pecos::GUMBEL: { // unbounded distribution
	Real mean, stdev;
	Pecos::moments_from_gumbel_params(guuv_alphas[guuv_cntr],
					  guuv_betas[guuv_cntr], mean, stdev);
	c_l_bnds[i] = mean - 10.*stdev; c_u_bnds[i] = mean + 10.*stdev;
	++guuv_cntr; break;
      }
      case Pecos::FRECHET: { // semibounded distribution
	Real mean, stdev;
	Pecos::moments_from_frechet_params(fuv_alphas[fuv_cntr],
					   fuv_betas[fuv_cntr], mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + 10.*stdev; ++fuv_cntr; break;
      }
      case Pecos::WEIBULL: { // semibounded distribution
	Real mean, stdev;
	Pecos::moments_from_weibull_params(wuv_alphas[wuv_cntr],
					   wuv_betas[wuv_cntr], mean, stdev);
	c_l_bnds[i] = 0.; c_u_bnds[i] = mean + 10.*stdev; ++wuv_cntr; break;
      }
      }
    }
    // uncertain bounds not currently used, since ACTIVE, not ACTIVE_UNIFORM
    g_u_model.continuous_lower_bounds(c_l_bnds);
    g_u_model.continuous_upper_bounds(c_u_bnds);
  }
}


void NonDExpansion::
construct_cubature(Iterator& u_space_sampler, Model& g_u_model,
		   unsigned short cub_int_order)
{
  expansionCoeffsApproach = Pecos::CUBATURE;
  u_space_sampler.assign_rep(new
    NonDCubature(g_u_model, natafTransform.u_types(), cub_int_order), false);
  numSamplesOnModel = u_space_sampler.maximum_concurrency()
                    / g_u_model.derivative_concurrency();
}


void NonDExpansion::
construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
		     const UShortArray& quad_order)
{
  expansionCoeffsApproach = Pecos::QUADRATURE;
  // define integration-dependent default refinement control
  if (stochExpRefineControl == Pecos::DEFAULT_CONTROL)
    stochExpRefineControl = Pecos::TOTAL_SOBOL;
  // enforce minimum required VBD control
  if (!vbdControl && stochExpRefineType == Pecos::ADAPTIVE_P_REFINEMENT &&
      stochExpRefineControl == Pecos::TOTAL_SOBOL)
    vbdControl = Pecos::UNIVARIATE_VBD;

  bool nested_rules = (ruleNestingOverride == Pecos::NESTED ||
    (ruleNestingOverride == Pecos::NO_OVERRIDE &&
     stochExpRefineType  != Pecos::NO_REFINEMENT));
  u_space_sampler.assign_rep(
    new NonDQuadrature(g_u_model, natafTransform.u_types(), quad_order,
		       nested_rules), false);
  numSamplesOnModel = u_space_sampler.maximum_concurrency()
                    / g_u_model.derivative_concurrency();
}


void NonDExpansion::
construct_sparse_grid(Iterator& u_space_sampler, Model& g_u_model,
		      unsigned short ssg_level, const RealVector& ssg_dim_pref)
{
  expansionCoeffsApproach = Pecos::SPARSE_GRID;
  // define integration-dependent default refinement control
  if (stochExpRefineControl == Pecos::DEFAULT_CONTROL)
    stochExpRefineControl = Pecos::GENERALIZED_SPARSE;
  // enforce minimum required VBD control
  if (!vbdControl && stochExpRefineType == Pecos::ADAPTIVE_P_REFINEMENT &&
      stochExpRefineControl == Pecos::TOTAL_SOBOL)
    vbdControl = Pecos::UNIVARIATE_VBD;

  //String sparse_grid_usage;
  //if (methodName == "nond_polynomial_chaos")
  //  sparse_grid_usage = "integration";
  //else if (methodName == "nond_stoch_collocation")
  //  sparse_grid_usage = "interpolation";

  bool nested_rules = (ruleNestingOverride != Pecos::NON_NESTED);

  u_space_sampler.assign_rep(
    new NonDSparseGrid(g_u_model, natafTransform.u_types(), ssg_level,
		       ssg_dim_pref, /* sparse_grid_usage, */ nested_rules,
		       stochExpRefineType, stochExpRefineControl), false);

  numSamplesOnModel = u_space_sampler.maximum_concurrency()
                    / g_u_model.derivative_concurrency();
}


void NonDExpansion::
construct_lhs(Iterator& u_space_sampler, Model& g_u_model)
{
  if (numSamplesOnModel <= 0) {
    Cerr << "Error: bad expansion construction specification in NonDExpansion."
	 << std::endl;
    abort_handler(-1);
  }

  /*
  if (exp_samples > 0 &&
      probDescDB.get_string("method.nond.expansion_sample_type")
      == "incremental_lhs")
    // TO DO: define and pass previous_samples ?
    u_space_sampler.assign_rep(new NonDIncremLHSSampling(g_u_model,
      numSamplesOnModel, orig_seed, ACTIVE), false);
  else
  */
  int orig_seed = probDescDB.get_int("method.random_seed");
  const String& rng = probDescDB.get_string("method.random_number_generator");
  u_space_sampler.assign_rep(new NonDLHSSampling(g_u_model, numSamplesOnModel,
			     orig_seed, rng, ACTIVE), false);
}


void NonDExpansion::initialize_u_space_model()
{
  // if all variables mode, initialize key to random variable subset
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  BoolDeque random_vars_key;
  size_t i;
  if (all_vars) {
    random_vars_key.resize(numContinuousVars);
    for (i=0; i<numContinuousVars; ++i)
      random_vars_key[i]
	= (i >= numContDesVars && i < numContDesVars + numContAleatUncVars);
  }
  // Specification data needed by PecosApproximation must be passed
  // by diving through the hierarchy.
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep;
  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep) { // may be NULL based on approxFnIndices
      poly_approx_rep->solution_approach(expansionCoeffsApproach);
      poly_approx_rep->refinement_control(stochExpRefineControl);
      poly_approx_rep->vbd_control(vbdControl);
      if (expansionCoeffsApproach == Pecos::CUBATURE ||
	  expansionCoeffsApproach == Pecos::QUADRATURE ||
	  expansionCoeffsApproach == Pecos::SPARSE_GRID)
	poly_approx_rep->integration_iterator(u_space_sampler);
      if (all_vars)
	poly_approx_rep->random_variables_key(random_vars_key);
    }
  }
}


void NonDExpansion::construct_expansion_sampler()
{
  if (totalLevelRequests) {
    //expSampling = false;
    for (size_t i=0; i<numFunctions; ++i)
      if ( requestedProbLevels[i].length() || requestedGenRelLevels[i].length()
	   || ( requestedRespLevels[i].length() &&
		respLevelTarget != RELIABILITIES ) )
	{ expSampling = true; break; }

    if (expSampling && !numSamplesOnExpansion) {
      Cerr << "\nError: number of samples must be specified for evaluating "
	   << "probabilities." << std::endl;
      abort_handler(-1);
    }

    // even if !expSampling, expansionSampler needed for moment projections
    int orig_seed = probDescDB.get_int("method.random_seed");
    const String& rng = probDescDB.get_string("method.random_number_generator");
    expansionSampler.assign_rep(new NonDLHSSampling(uSpaceModel,
      numSamplesOnExpansion, orig_seed, rng, UNCERTAIN), false);
    //expansionSampler.sampling_reset(numSamplesOnExpansion, true, false);
    NonD* exp_sampler_rep = (NonD*)expansionSampler.iterator_rep();
    exp_sampler_rep->requested_levels(requestedRespLevels, requestedProbLevels,
      requestedRelLevels, requestedGenRelLevels, respLevelTarget, cdfFlag);

    const String& integration_refine
      = probDescDB.get_string("method.nond.integration_refinement");
    if (!integration_refine.empty()) {
      impSampling = true;
      for (size_t i=0; i<numFunctions; i++) {
	if (!requestedProbLevels[i].empty() || !requestedRelLevels[i].empty() ||
	    !requestedGenRelLevels[i].empty()) {
	  Cerr << "\nError: importance sampling methods only supported for "
	       << "forward CDF/CCDF level mappings.\n\n";
	  abort_handler(-1);
	}
      }
    }

    if (impSampling) {
      short int_refine_type;
      if (integration_refine == "is")
	int_refine_type = IS;
      else if (integration_refine == "ais")
	int_refine_type = AIS;
      else if (integration_refine == "mmais")
	int_refine_type = MMAIS;
      Cout << "IS refinement type " << int_refine_type;

      int refine_samples = 1000;
      importanceSampler.assign_rep(new NonDAdaptImpSampling(uSpaceModel,
      	refine_samples, orig_seed, rng, int_refine_type, cdfFlag, false, false,
	false), false);
      uSpaceModel.init_communicators(importanceSampler.maximum_concurrency());
 
      NonDAdaptImpSampling* imp_sampler_rep = 
        (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
      imp_sampler_rep->requested_levels(requestedRespLevels,
	requestedProbLevels, requestedRelLevels, requestedGenRelLevels,
	respLevelTarget, cdfFlag);
    }
  }
}


void NonDExpansion::quantify_uncertainty()
{
  initialize_expansion();
  compute_expansion(); // nominal iso/aniso expansion from input spec

  // --------------------------------------
  // Uniform/adaptive refinement approaches
  // --------------------------------------
  if (stochExpRefineType) { // UNIFORM_P_REFINEMENT or ADAPTIVE_P_REFINEMENT

    size_t i, iter = 1;
    bool converged = (iter > maxIterations);
    Real metric;

    // post-process nominal expansion
    if (!converged)
      compute_print_iteration_results(true);

    // initialize refinement algorithms (if necessary)
    switch (stochExpRefineType) {
    case Pecos::ADAPTIVE_P_REFINEMENT:
      switch (stochExpRefineControl) {
      case Pecos::GENERALIZED_SPARSE:
	initialize_sets(); break;
      } break;
    }

    Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
    NonDIntegration* nond_integration
      = (NonDIntegration*)u_space_sampler.iterator_rep();
    while (!converged) {

      switch (stochExpRefineType) {
      case Pecos::UNIFORM_P_REFINEMENT:
	// Uniform refinement: ramp only the SSG level or TPQ order.
	// Note: initial specification can be isotropic or anisotropic.
	nond_integration->increment_grid(); // virtual: TPQ or SSG
	update_expansion();
        metric = compute_covariance_metric(respCovariance);
	break;
      case Pecos::ADAPTIVE_P_REFINEMENT:
	switch (stochExpRefineControl) {
	case Pecos::TOTAL_SOBOL: {
	  // Dimension adaptive refinement: define anisotropic preference vector
	  // from total Sobol' indices, averaged over response function set.
	  RealVector dim_pref;
	  average_total_sobol(dim_pref);
	  // incrementing grid & updating aniso wts are best performed together
	  nond_integration->increment_grid(dim_pref); // virtual: TPQ or SSG
	  update_expansion();
	  metric = compute_covariance_metric(respCovariance);
	  break;
	}
	case Pecos::SPECTRAL_DECAY: {
	  // Dimension adaptive refinement: define anisotropic preference vector
	  // from inverse of spectral decay rates (PCE only), averaged over
	  // response function set.
	  RealVector avg_decay, dim_pref(numContinuousVars, false);
	  average_decay_rates(avg_decay);
	  for (i=0; i<numContinuousVars; ++i)
	    dim_pref[i] = 1./avg_decay[i];
	  // incrementing grid & updating aniso wts are best performed together
	  nond_integration->increment_grid(dim_pref); // virtual: TPQ or SSG
	  update_expansion();
	  metric = compute_covariance_metric(respCovariance);
	  break;
	}
	case Pecos::GENERALIZED_SPARSE:
	  // Dimension adaptive refinement using generalized sparse grids.
	  // > Start GSG from iso/aniso SSG: starting from scratch (w=0) is most
	  //   efficient if fully nested; otherwise, unique points from lowest
	  //   level grids may not contribute (smolyak combinatorial coeff = 0).
	  // > Starting GSG from TPQ is conceptually straightforward but awkward
	  //   (would use nond_sparse->ssg_driver->compute_tensor_grid()).
	  metric = increment_sets(); // SSG only
	  break;
	}
	break;
      }

      converged = (metric <= convergenceTol || ++iter > maxIterations);
      if (!converged)
	compute_print_iteration_results(false);
      Cout << "\nRefinement iteration convergence metric = " << metric << '\n';
    }

    // finalize refinement algorithms (if necessary)
    switch (stochExpRefineType) {
    case Pecos::ADAPTIVE_P_REFINEMENT:
      switch (stochExpRefineControl) {
      case Pecos::GENERALIZED_SPARSE:
	finalize_sets(); break;
      } break;
    }
  }

  compute_print_converged_results();
  update_final_statistics();
  ++numUncertainQuant;
}


void NonDExpansion::compute_print_increment_results()
{
  switch (stochExpRefineType) {
  case Pecos::ADAPTIVE_P_REFINEMENT:
    switch (stochExpRefineControl) {
    case Pecos::GENERALIZED_SPARSE:
      if (totalLevelRequests) { // both covariance and full results available
	if (outputLevel == DEBUG_OUTPUT) print_results(Cout);
	//else                           print_covariance(Cout);
      }
      else { // only covariance available, compute full results if needed
	if (outputLevel == DEBUG_OUTPUT)
	  { compute_statistics(); print_results(Cout); }
	else
	  print_covariance(Cout);
      }
      break;
    }
    break;
  }
}


void NonDExpansion::compute_print_iteration_results(bool initialize)
{
#ifdef CONVERGENCE_DATA
  // full results compute/print mirroring Iterator::post_run(),
  // allowing output level to be set low in performance testing
  compute_statistics();
  iteratedModel.print_evaluation_summary(Cout);
  NonDExpansion::print_results(Cout);
  if (expansionCoeffsApproach == Pecos::SPARSE_GRID) {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    const UShort2DArray& sm_multi_index = nond_sparse->smolyak_multi_index();
    const IntArray&      sm_coeffs      = nond_sparse->smolyak_coefficients();
    size_t i, j, smi_len = sm_multi_index.size();
    for (i=0; i<smi_len; ++i) {
      if (sm_coeffs[i]) {
	Cout << "Smolyak index set " << i << ':';
	for (j=0; j<numContinuousVars; ++j)
	  Cout << ' ' << sm_multi_index[i][j];
	Cout << '\n';
      }
    }
  }
#else
  switch (stochExpRefineType) {
  case Pecos::UNIFORM_P_REFINEMENT:
    // In these cases, metric calculations already performed are still valid
    if (outputLevel == DEBUG_OUTPUT) // compute/print all stats
      { compute_statistics(); print_results(Cout); }
    else // compute/print subset of stats required for convergence assessment
      { if (initialize) compute_covariance(); print_covariance(Cout); }
    break;
  case Pecos::ADAPTIVE_P_REFINEMENT:
    switch (stochExpRefineControl) {
    case Pecos::TOTAL_SOBOL: case Pecos::SPECTRAL_DECAY:
      // In these cases, metric calculations already performed are still valid
      if (outputLevel == DEBUG_OUTPUT) // compute/print all stats
	{ compute_statistics(); print_results(Cout); }
      else // compute/print subset of stats required for convergence assessment
	{ if (initialize) compute_covariance(); print_covariance(Cout); }
      break;
    case Pecos::GENERALIZED_SPARSE:
      // In this case, the last index set calculation may not be the selected
      // index set. However, in the case where of non-debug output, we are using
      // partial results updating to eliminate the need to recompute stats for
      // the selected index set. This case also differs from other refinement
      // cases above in that compute_print_increment_results() has already
      // provided per increment output, so we do not push output if non-debug.
      if (totalLevelRequests) {
	if (initialize || outputLevel == DEBUG_OUTPUT) compute_statistics();
	if (outputLevel == DEBUG_OUTPUT)               print_results(Cout);
	// else no output!
      }
      else {
	if (outputLevel == DEBUG_OUTPUT)               compute_statistics();
	else if (initialize)                           compute_covariance();
	if (outputLevel == DEBUG_OUTPUT)               print_results(Cout);
	// else no output!
      }
      break;
    }
    break;
  }
#endif // CONVERGENCE_DATA
}


void NonDExpansion::compute_print_converged_results()
{
#ifdef CONVERGENCE_DATA
  if (expansionCoeffsApproach == Pecos::SPARSE_GRID) {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    const UShort2DArray& sm_multi_index = nond_sparse->smolyak_multi_index();
    const IntArray&      sm_coeffs      = nond_sparse->smolyak_coefficients();
    size_t i, j, smi_len = sm_multi_index.size();
    for (i=0; i<smi_len; ++i) {
      if (sm_coeffs[i]) {
	Cout << "Smolyak index set " << i << ':';
	for (j=0; j<numContinuousVars; ++j)
	  Cout << ' ' << sm_multi_index[i][j];
	Cout << '\n';
      }
    }
  }
#endif

  // if not already performed above, compute all stats
  switch (stochExpRefineType) {
  case Pecos::NO_REFINEMENT:
    compute_statistics();
    break;
  case Pecos::UNIFORM_P_REFINEMENT:
    if (outputLevel != DEBUG_OUTPUT)
      compute_statistics();
    break;
  case Pecos::ADAPTIVE_P_REFINEMENT:
    switch (stochExpRefineControl) {
    case Pecos::TOTAL_SOBOL: case Pecos::SPECTRAL_DECAY:
      if (outputLevel != DEBUG_OUTPUT)
	compute_statistics();
      break;
    case Pecos::GENERALIZED_SPARSE:
      compute_statistics();
      break;
    }
    break;
  }

  // For stand-alone executions, print_results occurs in Iterator::post_run().
  // For sub-iterator executions, stats are normally suppressed.
  if (subIteratorFlag && outputLevel == DEBUG_OUTPUT)
    print_results(Cout);
}


void NonDExpansion::initialize_sets()
{
  Cout << "\n>>>>> Initialization of generalized sparse grid sets.\n";
  NonDSparseGrid* nond_sparse
    = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
  nond_sparse->initialize_sets();
  nond_sparse->update_reference();
}


Real NonDExpansion::increment_sets()
{
  Cout << "\n>>>>> Begin evaluation of active index sets.\n";

  NonDSparseGrid* nond_sparse
    = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
  std::set<UShortArray>::const_iterator cit, cit_star;
  Real delta, delta_star = -1.;
  RealSymMatrix covar_ref, covar_star; RealVector stats_ref, stats_star;

  // set reference points for refinement assessment
  if (totalLevelRequests) stats_ref = finalStatistics.function_values();
  else                    covar_ref = respCovariance;

  // Reevaluate the effect of every active set every time, since the reference
  // point for the surplus calculation changes (and the overlay should
  // eventually be inexpensive since each point set is only evaluated once).
  const std::set<UShortArray>& active_mi = nond_sparse->active_multi_index();
  for (cit=active_mi.begin(); cit!=active_mi.end(); ++cit) {

    // increment grid with current candidate
    Cout << "\n>>>>> Evaluating trial index set:\n" << *cit;
    nond_sparse->increment_set(*cit);
    if (uSpaceModel.restore_available())      // has been active previously
      uSpaceModel.restore_approximation();
    else {                                    // a new active set
      nond_sparse->evaluate_set();
      uSpaceModel.append_approximation(true); // rebuild
    }

    // assess effect of increment (non-negative norm) and store best
    delta = (totalLevelRequests) ? compute_final_statistics_metric(stats_ref)
                                 : compute_covariance_metric(covar_ref); 
    if (delta > delta_star) {
      cit_star = cit; delta_star = delta;
      // partial results tracking avoids need to recompute statistics
      // on the selected index set
      if (outputLevel < DEBUG_OUTPUT) {
	if (totalLevelRequests) stats_star = finalStatistics.function_values();
	else                    covar_star = respCovariance;
      }
    }
    compute_print_increment_results();
    Cout << "\n<<<<< Trial set refinement metric = " << delta << '\n';

    // restore previous state (destruct order is reversed from construct order)
    uSpaceModel.pop_approximation();
    nond_sparse->decrement_set();
  }
  Cout << "\n<<<<< Evaluation of active index sets completed.\n"
       << "\n<<<<< Index set selection:\n" << *cit_star;

  // permanently apply best increment and update ref points for next increment
  nond_sparse->update_sets(*cit_star);
  uSpaceModel.restore_approximation();
  nond_sparse->update_reference();
  if (outputLevel < DEBUG_OUTPUT) { // partial results tracking
    if (totalLevelRequests) finalStatistics.function_values(stats_star);
    else                    respCovariance = covar_star;
  }

  return delta_star;
}


void NonDExpansion::finalize_sets()
{
  Cout << "\n<<<<< Finalization of generalized sparse grid sets.\n";
  NonDSparseGrid* nond_sparse
    = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
  // apply all remaining increments not previously selected
  nond_sparse->finalize_sets();
  uSpaceModel.finalize_approximation();
}


Real NonDExpansion::
compute_covariance_metric(const RealSymMatrix& resp_covar_ref)
{
  RealSymMatrix delta_resp_covar = resp_covar_ref;
  compute_covariance();
  delta_resp_covar -= respCovariance;
  return delta_resp_covar.normFrobenius();
}


/** computes a "goal-oriented" refinement metric employing finalStatistics */
Real NonDExpansion::
compute_final_statistics_metric(const RealVector& final_stats_ref)
{
  RealVector delta_final_stats = final_stats_ref;
  compute_statistics();
  delta_final_stats -= finalStatistics.function_values();
#ifdef DEBUG
  Cout << "final_stats_ref:\n" << final_stats_ref
       << "final_stats:\n" << finalStatistics.function_values()
       << "delta_final_stats:\n" << delta_final_stats << std::endl;
#endif // DEBUG

  // sum up only the level mapping stats (don't mix with mean and variance due
  // to scaling issues)
  // TO DO: if the level mappings are of mixed type, then would need to scale
  //        with a target value or measure norm of relative change.
  Real sum_sq = 0.; size_t i, j, cntr = 0, num_levels_i;
  for (i=0; i<numFunctions; ++i) {
    cntr += 2; // skip moments
    num_levels_i = requestedRespLevels[i].length() +
      requestedProbLevels[i].length() + requestedRelLevels[i].length() +
      requestedGenRelLevels[i].length();
    for (j=0; j<num_levels_i; ++j, ++cntr)
      sum_sq += std::pow(delta_final_stats[cntr], 2.);
  }

  return std::sqrt(sum_sq);
}


void NonDExpansion::average_total_sobol(RealVector& avg_sobol)
{
  // anisotropy based on total Sobol indices (univariate effects only) averaged
  // over the response fn set.  [Addition of interaction effects based on
  // individual Sobol indices would require a nonlinear index set constraint
  // within anisotropic sparse grids.]
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  size_t i;
  if (avg_sobol.empty())
    avg_sobol.sizeUninitialized(numContinuousVars);
  avg_sobol = 0.;
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (vbdControl == Pecos::ALL_VBD)
      approx_rep->compute_component_effects(); // needed for total effects sum
    approx_rep->compute_total_effects();
    avg_sobol += approx_rep->total_sobol_indices();
  }
  avg_sobol.scale(1./(Real)numFunctions);
  //print_total_effects(Cout);
  // manage small values that are not 0 (SGMGA has special handling for 0)
  Real pref_tol = 1.e-2; // TO DO
  for (i=0; i<numContinuousVars; ++i)
    if (std::abs(avg_sobol[i]) < pref_tol)
      avg_sobol[i] = 0.;
  //Cout << "avg_sobol truncated at " << pref_tol << ":\n";
  //write_data(Cout, avg_sobol);
}


void NonDExpansion::average_decay_rates(RealVector& avg_decay)
{
  // "main effects" anisotropy based on linear approximation to coefficient
  // decay rates for each dimension as measured from univariate PCE terms.
  // Again, these rates are averaged over the response fn set.
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  size_t i;
  if (avg_decay.empty())
    avg_decay.sizeUninitialized(numContinuousVars);
  avg_decay = 0.;
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    // TO DO: normalize coefficients by multiplying by sqrt(norm_sq)
    // TO DO: Richardson extrapolation for best fit polynomial order
    // TO DO: invert order for preference
    //approx_rep->compute_dimension_decay rates();
    //avg_decay += approx_rep->dimension_decay_rates();
  }
  avg_decay.scale(1./(Real)numFunctions);
  // manage small values that are not 0 (SGMGA has special handling for 0)
  Real pref_tol = 1.e-2; // TO DO
  for (i=0; i<numContinuousVars; ++i)
    if (std::abs(avg_decay[i]) < pref_tol)
      avg_decay[i] = 0.;
  Cout << "avg_decay truncated at " << pref_tol << ":\n";
  write_data(Cout, avg_decay);
}


void NonDExpansion::initialize_expansion()
{
  // update ranVar info to capture any distribution param insertions
  initialize_random_variable_parameters();
  initialize_final_statistics_gradients();
  natafTransform.trans_correlations();

  // now that labels have flowed down at run-time from any higher level
  // recursions, propagate them up the instantiate-on-the-fly Model
  // recursion so that they are correct when they propagate back down.
  uSpaceModel.update_from_subordinate_model(); // recurse_flag = true

  // propagate latest natafTransform settings to u-space sampler
  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  if (!u_space_sampler.is_null())
    ((NonD*)u_space_sampler.iterator_rep())->
      initialize_random_variables(natafTransform);

  // store the current design/state vars in u-space
  size_t i, j, cntr = 0;
  RealVector initial_pt_x;
  if (numContDesVars || numContEpistUncVars || numContStateVars ||
      !subIteratorFlag){
    copy_data(iteratedModel.continuous_variables(), initial_pt_x); // view->copy
    if (numUncertainQuant) { // reset uncertain values to means
      const Pecos::RealVector& x_means = natafTransform.x_means();
      for (i=numContDesVars; i<numContDesVars + numContAleatUncVars; ++i)
	initial_pt_x[i] = x_means[i];
    }
    natafTransform.trans_X_to_U(initial_pt_x, initialPtU);
  }
}


void NonDExpansion::compute_expansion()
{
#ifdef DERIV_DEBUG
  // numerical verification of analytic Jacobian/Hessian routines
  RealVector rdv_u;
  natafTransform.trans_X_to_U(iteratedModel.continuous_variables(), rdv_u);
  natafTransform.verify_trans_jacobian_hessian(rdv_u);//(rdv_x);
  natafTransform.verify_design_jacobian(rdv_u);
#endif // DERIV_DEBUG

  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  NonD* u_space_sampler_rep = (NonD*)u_space_sampler.iterator_rep();

  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  size_t i, j, num_final_stats = final_asv.size(),
    num_final_grad_vars = final_dvv.size(), cntr = 0;
  bool final_stat_value_flag = false, final_stat_grad_flag = false;
  for (i=0; i<num_final_stats; ++i)
    if (final_asv[i] & 1)
      { final_stat_value_flag = true; break; }
  for (i=0; i<num_final_stats; ++i)
    if (final_asv[i] & 2)
      { final_stat_grad_flag  = true; break; }

  // define active set request vector for u_space_sampler
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  ActiveSet sampler_set;
  ShortArray sampler_asv(numFunctions, 0);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    bool expansion_coeff_flag = false, expansion_grad_flag = false,
         mean_grad_flag       = false, std_dev_grad_flag   = false;
    size_t rl_len = requestedRespLevels[i].length(),
	   pl_len = requestedProbLevels[i].length(),
	   bl_len = requestedRelLevels[i].length(),
	   gl_len = requestedGenRelLevels[i].length();

    if (final_stat_value_flag)
      // map final_asv value bits into expansion_coeff_flag requirements
      for (j=0; j<2+rl_len+pl_len+bl_len+gl_len; ++j)
	if (final_asv[cntr+j] & 1)
	  { expansion_coeff_flag = true; break; }

    if (final_stat_grad_flag) {
      // map final_asv gradient bits into moment grad requirements
      if (final_asv[cntr++] & 2)
	mean_grad_flag    = true;
      if (final_asv[cntr++] & 2)
	std_dev_grad_flag = true;
      if (respLevelTarget == RELIABILITIES)
	for (j=0; j<rl_len; ++j)
	  if (final_asv[cntr+j] & 2)
	    { mean_grad_flag = std_dev_grad_flag = true; break; }
      cntr += rl_len + pl_len;
      for (j=0; j<bl_len; ++j)
	if (final_asv[cntr+j] & 2)
	  { mean_grad_flag = std_dev_grad_flag = true; break; }
      cntr += bl_len + gl_len;
      // map moment grad requirements into expansion_{coeff,grad}_flag reqmts
      // (refer to *PolyApproximation::get_*_gradient() implementations)
      if (all_vars) { // aleatory + design/epistemic
	if (std_dev_grad_flag)
	  expansion_coeff_flag = true;
	size_t deriv_index, num_deriv_vars = final_dvv.size();
	for (j=0; j<num_deriv_vars; ++j) {
	  deriv_index = final_dvv[j] - 1; // OK since we are in an "All" view
	  if (deriv_index >= numContDesVars &&
	      deriv_index <  numContDesVars + numContAleatUncVars) { // ran var
	    if (mean_grad_flag || std_dev_grad_flag)
	      expansion_grad_flag = true;
	  }
	  else if (mean_grad_flag)
	    expansion_coeff_flag = true;
	}
      }
      else { // aleatory expansion variables
	if (mean_grad_flag)
	  expansion_grad_flag = true;
	if (std_dev_grad_flag)
	  expansion_coeff_flag = expansion_grad_flag = true;
      }
    }
    else
      cntr += 2 + rl_len + pl_len + bl_len + gl_len;

    // map expansion_{coeff,grad}_flag requirements into ASV and
    // PecosApproximation settings
    if (expansion_coeff_flag)
      sampler_asv[i] |= 1;
    if (expansion_grad_flag || useDerivsFlag)
      sampler_asv[i] |= 2;
    poly_approx_rep->expansion_coefficient_flag(expansion_coeff_flag);
    poly_approx_rep->expansion_gradient_flag(expansion_grad_flag);
  }

  // If OUU/SOP (multiple calls to quantify_uncertainty()), an expansion
  // constructed over the full range of all variables does not need to be
  // reconstructed on subsequent calls.  However, an all_vars construction
  // over a trust region needs rebuilding when the trust region is updated.
  // In the checks below, all_approx detects any variable insertions or ASV
  // omissions and force_rebuild() manages variable augmentations.
  bool all_approx = false;
  if (all_vars && numUncertainQuant && secondaryACVarMapTargets.empty()) {
    all_approx = true;
    // does sampler_asv contain content not evaluated previously
    const ShortArray& prev_asv = u_space_sampler.active_set().request_vector();
    for (i=0; i<numFunctions; ++i)
      // bit-wise AND checks if each sampler_asv bit is present in prev_asv
      if ( (prev_asv[i] & sampler_asv[i]) != sampler_asv[i] )
	{ all_approx = false; break; }
  }
  if (!all_approx || uSpaceModel.force_rebuild()) {
    sampler_set.request_vector(sampler_asv);

    // if required statistical sensitivities are not covered by All variables
    // mode for augmented design variables, then the simulations must evaluate
    // response sensitivities.
    bool sampler_grad = false, dist_param_deriv = false;
    if (final_stat_grad_flag) {
      size_t i, num_outer_cv = secondaryACVarMapTargets.size();
      for (i=0; i<num_outer_cv; ++i)
	if (secondaryACVarMapTargets[i] != Pecos::NO_TARGET) // insertion
	  { dist_param_deriv = true; break; }
      sampler_grad = (all_vars) ? dist_param_deriv : true;
    }
    u_space_sampler_rep->distribution_parameter_derivatives(dist_param_deriv);
    if (dist_param_deriv)
      u_space_sampler.active_variable_mappings(primaryACVarMapIndices,
	primaryADIVarMapIndices, primaryADRVarMapIndices,
	secondaryACVarMapTargets, secondaryADIVarMapTargets,
	secondaryADRVarMapTargets);

    // final_dvv: NestedModel::derived_compute_response() maps the top-level
    // optimizer deriv vars to the sub-iterator deriv vars in
    // NestedModel::set_mapping() and then sets this DVV within finalStats
    // using subIterator.response_results_active_set().  NonDPCE/NonDSC then
    // maps the finalStats DVV to the default set for u_space_sampler, which
    // may then be augmented for correlations in NonD::set_u_to_x_mapping().
    if (all_vars && sampler_grad) {
      SizetArray filtered_final_dvv; // retain insertion targets only
      for (i=0; i<num_final_grad_vars; ++i) {
	size_t dvv_i = final_dvv[i];
	if (dvv_i >  numContDesVars &&
	    dvv_i <= numContDesVars+numContAleatUncVars)
	  filtered_final_dvv.push_back(dvv_i);
      }
      sampler_set.derivative_vector(filtered_final_dvv);
    }
    else
      sampler_set.derivative_vector(final_dvv);

    // Build the orthogonal/interpolation polynomial approximations:
    u_space_sampler.active_set(sampler_set);
    uSpaceModel.build_approximation();
  }
}


void NonDExpansion::update_expansion()
{
  // leave sampler_set, expansion flags, and distribution parameter
  // settings as set previously by compute_expansion(); there should
  // be no need to update for an expansion refinement.

  // Ultimately want to be more hierarchical than this; this is just
  // a first step for now:
  uSpaceModel.build_approximation();
  //u_space_sampler->run_iterator(Cout);
  //uSpaceModel.update_approximation(true); // append_approximation() ?
}


void NonDExpansion::compute_covariance()
{
  if (respCovariance.empty())
    respCovariance.shapeUninitialized(numFunctions);

  size_t i, j;
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep_i
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (!poly_approx_rep_i->expansion_coefficient_flag()) {
      Cerr << "Error: expansion coefficients required in NonDExpansion::"
	   << "compute_moments()." << std::endl;
      abort_handler(-1);
    }

    if (all_vars) { // TO DO: for now, define diagonal only
      respCovariance(i,i) = poly_approx_rep_i->get_variance(initialPtU);
      for (j=0; j<i; ++j) // TO DO: compute all_vars covariance at initialPtU
	respCovariance(i,j) = 0.;
    }
    else
      for (j=0; j<=i; ++j)
	respCovariance(i,j) = poly_approx_rep_i->get_covariance(
	  poly_approxs[j].approximation_coefficients());
  }
}


/** Calculate analytic and numerical statistics from the expansion and
    log results within final_stats for use in OUU. */
void NonDExpansion::compute_statistics()
{
  // -----------------------------
  // Calculate analytic statistics
  // -----------------------------
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  const SizetArray& final_dvv = finalStatistics.active_set_derivative_vector();
  bool all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
  size_t i, j, k, cntr = 0, num_final_grad_vars = final_dvv.size();

  // initialize expGradsMeanX
  if (!subIteratorFlag && expGradsMeanX.empty())
    expGradsMeanX.shapeUninitialized(numContinuousVars, numFunctions);

  // compute response covariance
  bool covariance_flag = (!subIteratorFlag || (stochExpRefineType &&
			  stochExpRefineControl != Pecos::GENERALIZED_SPARSE));
  if (covariance_flag)
    compute_covariance();

  // loop over response fns and compute/store analytic stats/stat grads
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  betaMappings = false;
  for (i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();

    // store mean/std dev grads in finalStatistics if needed for beta mappings
    bool beta_mapping_flag = false, beta_mapping_grad_flag = false;
    if (respLevelTarget == RELIABILITIES && !requestedRespLevels[i].empty()) {
      size_t rl_len = requestedRespLevels[i].length();
      for (j=0; j<rl_len; ++j) {
	short asv_j = final_asv[cntr+2+j];
	if (asv_j & 3) beta_mapping_flag      = true;
	if (asv_j & 2) beta_mapping_grad_flag = true;
      }
    }
    if (!requestedRelLevels[i].empty()) {
      size_t bl_len = requestedRelLevels[i].length(),
	     pl_len = requestedProbLevels[i].length();
      for (j=0; j<bl_len; ++j) {
	short asv_j = final_asv[cntr+2+pl_len+j];
	if (asv_j & 1) beta_mapping_flag      = true;
	if (asv_j & 2) beta_mapping_grad_flag = true;
      }
    }
    if (beta_mapping_flag)
      betaMappings = true;

    // *** analytic mean
    if (final_asv[cntr] & 1 || beta_mapping_flag) {
      meanStats[i] = (all_vars) ? 
	poly_approx_rep->get_mean(initialPtU) :
	poly_approx_rep->get_mean();
      if (final_asv[cntr] & 1)
	finalStatistics.function_value(meanStats[i], cntr);
    }
    // *** analytic mean gradient
    if (final_asv[cntr] & 2 || beta_mapping_grad_flag) {
      const RealVector& final_stat_grad = (all_vars) ?
	poly_approx_rep->get_mean_gradient(initialPtU, final_dvv) :
	poly_approx_rep->get_mean_gradient();
      finalStatistics.function_gradient(final_stat_grad, cntr);
    }
    cntr++;

    // *** analytic std deviation
    if (final_asv[cntr] & 1 || beta_mapping_flag) {
      const Real& variance = (covariance_flag) ? respCovariance(i,i) :
	((all_vars) ? poly_approx_rep->get_variance(initialPtU)
	            : poly_approx_rep->get_variance());
      if (variance >= 0.)
	stdDevStats[i] = std::sqrt(variance);
      else { // negative variance can happen with SC on sparse grids
	Cerr << "Warning: stochastic expansion variance is negative in "
	     << "computation of std deviation.\n         Setting std deviation "
	     << "to zero." << std::endl;
	stdDevStats[i] = 0.;
      }
      if (final_asv[cntr] & 1)
	finalStatistics.function_value(stdDevStats[i], cntr);
    }
    // *** analytic std deviation gradient
    if (final_asv[cntr] & 2 || beta_mapping_grad_flag) {
      RealVector final_stat_grad = (all_vars) ?
	poly_approx_rep->get_variance_gradient(initialPtU, final_dvv) :
	poly_approx_rep->get_variance_gradient();
      const Real& sigma = stdDevStats[i];
      if (sigma > 0.)
	for (j=0; j<num_final_grad_vars; ++j)
	  final_stat_grad[j] /= 2.*sigma;
      else {
	Cerr << "Warning: stochastic expansion std deviation is zero in "
	     << "computation of std deviation gradient.\n         Setting "
	     << "gradient to zero." << std::endl;
	final_stat_grad = 0.;
      }
      finalStatistics.function_gradient(final_stat_grad, cntr);
    }
    cntr++;

    // *** local sensitivities
    if (!subIteratorFlag && poly_approx_rep->expansion_coefficient_flag()) {
      // expansion sensitivities are defined from the coefficients and basis
      // polynomial derivatives.  They are computed for the means of the
      // uncertain varables and are intended to serve as importance factors.
      const RealVector& exp_grad_u_rv
	= poly_approxs[i].get_gradient(initialPtU);
      RealVector exp_grad_u_pv, exp_grad_x_pv;
      copy_data(exp_grad_u_rv, exp_grad_u_pv);
      SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
      SizetArray x_dvv; copy_data(cv_ids, x_dvv);
      natafTransform.trans_grad_U_to_X(exp_grad_u_pv, exp_grad_x_pv,
				       natafTransform.x_means(), x_dvv, cv_ids);
      Teuchos::setCol(exp_grad_x_pv, (int)i, expGradsMeanX);
    }

    // *** global sensitivities
    // Note: VBD needed for sub-iterator with Sobol refinement
    if (vbdControl && poly_approx_rep->expansion_coefficient_flag()) {
      poly_approx_rep->compute_component_effects();
      poly_approx_rep->compute_total_effects();
    }

    cntr += requestedRespLevels[i].length() + requestedProbLevels[i].length()
         +  requestedRelLevels[i].length()  + requestedGenRelLevels[i].length();
  }

  // ------------------------------
  // Calculate numerical statistics
  // ------------------------------
  // Estimate CDF/CCDF statistics by sampling on the expansion
  if (totalLevelRequests) {
    NonDSampling* exp_sampler_rep
      = (NonDSampling*)expansionSampler.iterator_rep();
    
    NonDAdaptImpSampling* imp_sampler_rep
      = (NonDAdaptImpSampling*)importanceSampler.iterator_rep();
  
    if (expSampling) { // sample on expansion to generate probability mappings
      // pass x-space data so that u-space Models can perform inverse transforms
      exp_sampler_rep->initialize_random_variables(natafTransform);
      // since expansionSampler uses an UNCERTAIN sampling mode, we must set the
      // unsampled variables to their u-space values.
      if (numContDesVars || numContEpistUncVars || numContStateVars)
	uSpaceModel.continuous_variables(initialPtU);
      // response fn is active for z->p, z->beta*, p->z, or beta*->z
      ActiveSet sampler_set = expansionSampler.active_set(); // copy
      ShortArray sampler_asv(numFunctions, 0);
      cntr = 0;
      for (i=0; i<numFunctions; ++i) {
	cntr += 2;
	size_t rl_len = requestedRespLevels[i].length();
	if (respLevelTarget != RELIABILITIES)
	  for (j=0; j<rl_len; ++j)
	    if (final_asv[cntr+j] & 1)
	      { sampler_asv[i] |= 1; break; }
	cntr += rl_len;
	size_t pl_len = requestedProbLevels[i].length();
	for (j=0; j<pl_len; ++j)
	  if (final_asv[cntr+j] & 1)
	    { sampler_asv[i] |= 1; break; }
	cntr += pl_len + requestedRelLevels[i].length();
	size_t gl_len = requestedGenRelLevels[i].length();
	for (j=0; j<gl_len; ++j)
	  if (final_asv[cntr+j] & 1)
	    { sampler_asv[i] |= 1; break; }
	cntr += gl_len;
      }
      sampler_set.request_vector(sampler_asv);
      expansionSampler.active_set(sampler_set);
      // no summary output since on-the-fly constructed:
      expansionSampler.run_iterator(Cout);
    }

    if (betaMappings)
      exp_sampler_rep->moments(meanStats, stdDevStats); // for beta mappings
    if (betaMappings || expSampling) {
      exp_sampler_rep->
	compute_distribution_mappings(expansionSampler.all_responses());
      exp_sampler_rep->update_final_statistics();
    }
    // sample correlations are superceded by analytic VBD
    //if (!subIteratorFlag && expSampling)
    //  exp_sampler_rep->compute_correlations(expansionSampler.all_variables(),
    //					      expansionSampler.all_responses());
    const RealVector& sampler_final_stats
      = expansionSampler.response_results().function_values();
 
    RealVectorArray imp_final_stats(numFunctions);
    if (impSampling) {
      //imp_sampler_rep->initialize_random_variables(natafTransform);
      // since importanceSampler uses an UNCERTAIN sampling mode, we must set
      // the unsampled variables to their u-space values.
      //if (numContDesVars || numContEpistUncVars || numContStateVars)
      //uSpaceModel.continuous_variables(initialPtU);
      // response fn is active for z->p, z->beta*, p->z, or beta*->z
      //ActiveSet sampler_set = importanceSampler.active_set(); // copy
      //ShortArray sampler_asv(numFunctions, 0);
      cntr = 0;

      const RealMatrix&    exp_vars      = expansionSampler.all_samples();
      const ResponseArray& exp_responses = expansionSampler.all_responses();
      int exp_cv = exp_vars.numRows();

      for (i=0; i<numFunctions; ++i) {
	cntr += 2;
	int rl_len = requestedRespLevels[i].length();
        imp_final_stats[i].resize(rl_len);
	if (respLevelTarget != RELIABILITIES)
	  for (j=0; j<rl_len; j++) {
            // Currently initializing importance sampling with both 
            // original build points and LHS expansion sampler points
	    const SDPList& expansion_data = uSpaceModel.approximation_data(i);
	    size_t num_data_pts = expansion_data.size();
            size_t num_to_is = numSamplesOnExpansion + num_data_pts;
           
            RealVectorArray initial_points(num_to_is);
	    //RealPRPMultiMap bestMap;
            
            //for (int k = 0; k<numSamplesOnExpansion;k++) {
	      //ParamResponsePair curr_prp(exp_vars[k],
	      //  iteratedModel.interface_id(), exp_responses[k],k);
	      //Real fn_value = exp_responses[k].function_values()[i];
	      //Real closeThreshold = fabs(fn_value-requestedRespLevels[i][j]);
	      //Cout << "fn_value " << fn_value << "\ncloseThreshold "
	      //     << closeThreshold << '\n';

	      //if (k < numImpStartPoints) {
	        //bestMap.insert(
	        //  std::pair<Real,ParamResponsePair>(closeThreshold,curr_prp));
	      //}
	      //else {
	        //RealPRPMultiMap::iterator last_iter = --bestMap.end();
	        //if (closeThreshold < last_iter->first){
	        //  bestMap.erase(last_iter);
	        //  bestMap.insert(std::pair<Real,ParamResponsePair>(
	        //                 closeThreshold, curr_prp));
	        //}
	      //}
	    //}
	    //RealPRPMultiMap::iterator iter;
            size_t m;

	    //for (m=0, iter=bestMap.begin();
	    //     m<numImpStartPoints && iter!=bestMap.end(); ++iter, ++m)
	    SDPLCIter cit = expansion_data.begin();
	    for (m=0; m<num_data_pts; ++m, ++cit) {
	      initial_points[m] = cit->continuous_variables(); // view OK
	      //Cout << "InitialPoint M" << initial_points[m];  
	    }

            for (m=0; m<numSamplesOnExpansion; m++) {
	      // ParamResponsePair prp = iter->second;
	      //initial_points[m]=prp.prp_parameters().continuous_variables();
              copy_data(exp_vars[m], exp_cv, initial_points[m+num_data_pts]);
	      //Cout << "InitialPoint M + buildpoints"
	      //     << initial_points[m+num_data_pts] << "Responses at M"
	      //     << prp.prp_response() << "\nResponses at M"
	      //     << exp_responses[m] << '\n';
	    }

            Cout << "Initial estimate of p to seed  "
		 << sampler_final_stats[cntr+j] << "\n";

	    imp_sampler_rep->initialize(initial_points, i, 
              sampler_final_stats[cntr+j], requestedRespLevels[i][j]);
          
	    // no summary output since on-the-fly constructed:
            importanceSampler.run_iterator(Cout); 
            const Real& p = imp_sampler_rep->get_probability();
            //Cout << "importance sampling estimate for function " << i 
            //     << " level " << j << " = " << p << "\n";
	    imp_final_stats[i][j]=p;

	    //bestMap.clear();
        }
      }
    }
 
    RealVector final_stat_grad;
    if (!finalStatistics.function_gradients().empty())
      final_stat_grad.sizeUninitialized(num_final_grad_vars);
    cntr = 0;
    for (i=0; i<numFunctions; ++i) {//level stats provided from expansionSampler
      size_t rl_len = requestedRespLevels[i].length(),
	     pl_len = requestedProbLevels[i].length(),
	     bl_len = requestedRelLevels[i].length(),
	     gl_len = requestedGenRelLevels[i].length();

      bool beta_mapping_grad_flag = false;
      if (respLevelTarget == RELIABILITIES)
	for (j=0; j<rl_len; ++j)
	  if (final_asv[cntr+2+j] & 2)
	    { beta_mapping_grad_flag = true; break; }
      for (j=0; j<bl_len; ++j)
	if (final_asv[cntr+2+pl_len+j] & 2)
	  { beta_mapping_grad_flag = true; break; }
      RealVector empty_rv;
      RealVector mean_grad    = (beta_mapping_grad_flag) ?
	finalStatistics.function_gradient(cntr) : empty_rv;
      RealVector std_dev_grad = (beta_mapping_grad_flag) ?
	finalStatistics.function_gradient(cntr+1) : empty_rv;
      cntr += 2;

      for (j=0; j<rl_len; j++, cntr++) {
	if (final_asv[cntr] & 1) {
          if (impSampling)
	    finalStatistics.function_value(imp_final_stats[i][j], cntr);
	  else
            finalStatistics.function_value(sampler_final_stats[cntr], cntr);
        }
	if (final_asv[cntr] & 2) {
	  switch (respLevelTarget) {
	  case PROBABILITIES: // TO DO: z->p sampling sensitivity analysis
	    Cerr << "\nError: analytic response probability sensitivity not "
		 << "yet supported." << std::endl;
	    abort_handler(-1);
	    break;
	  case RELIABILITIES: {
	    const Real& std_dev = stdDevStats[i];
	    if (std_dev > 1.e-25) {
	      const Real& z_bar = requestedRespLevels[i][j];
	      const Real& mean  = meanStats[i];
	      for (k=0; k<num_final_grad_vars; ++k) {
		//Real ratio = (meanStats[i] - z_bar)/stdDevStats[i];
		Real dratio_dx = (std_dev*mean_grad[k] - (mean - z_bar)*
				  std_dev_grad[k]) / std::pow(std_dev, 2);
		final_stat_grad[k] = (cdfFlag) ? dratio_dx : -dratio_dx;
	      }
	    }
	    else
	      final_stat_grad = 0.;
	    break;
	  }
	  case GEN_RELIABILITIES: // TO DO: z->p->beta* sampling SA
	    Cerr << "\nError: analytic response generalized reliability "
		 << "sensitivity not yet supported." << std::endl;
	    abort_handler(-1);
	    break;
	  }
	  finalStatistics.function_gradient(final_stat_grad, cntr);
	}
      }
      for (j=0; j<pl_len; j++, cntr++) {
	if (final_asv[cntr] & 1)
	  finalStatistics.function_value(sampler_final_stats[cntr], cntr);
	if (final_asv[cntr] & 2) {
	  // TO DO: p->z sampling sensitivity analysis
	  Cerr << "\nError: analytic response level sensitivity not yet "
	       << "supported for mapping from probability." << std::endl;
	  abort_handler(-1);
	}
      }
      for (j=0; j<bl_len; j++, cntr++) {
	if (final_asv[cntr] & 1)
	  finalStatistics.function_value(sampler_final_stats[cntr], cntr);
	if (final_asv[cntr] & 2) {
	  const Real& beta_bar = requestedRelLevels[i][j];
	  for (k=0; k<num_final_grad_vars; ++k)
	    final_stat_grad[k] = (cdfFlag) ?
	      mean_grad[k] - beta_bar * std_dev_grad[k] :
	      mean_grad[k] + beta_bar * std_dev_grad[k];

	  finalStatistics.function_gradient(final_stat_grad, cntr);
	}
      }
      for (j=0; j<gl_len; j++, cntr++) {
	if (final_asv[cntr] & 1)
	  finalStatistics.function_value(sampler_final_stats[cntr], cntr);
	if (final_asv[cntr] & 2) {
	  // TO DO: beta*->p->z sampling sensitivity analysis
	  Cerr << "\nError: analytic response level sensitivity not yet "
	       << "supported for mapping from generalized reliability."
	       << std::endl;
	  abort_handler(-1);
	}
      }
    }
  }
}


void NonDExpansion::update_final_statistics()
{
  if (!finalStatistics.function_gradients().empty()) {
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

    // for all_variables, finalStatistics design grads are in extended u-space
    // -> transform to the original design space
    if (numContDesVars || numContEpistUncVars || numContStateVars) {
      // this approach is more efficient but less general.  If we can assume
      // that the DVV only contains design/state vars, then we know they are
      // uncorrelated and the jacobian matrix is diagonal with terms 2./range.
      SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
      const SizetArray& final_dvv
	= finalStatistics.active_set_derivative_vector();
      size_t num_final_grad_vars = final_dvv.size();
      Real factor;
      const Pecos::RealVector& x_l_bnds = natafTransform.x_lower_bounds();
      const Pecos::RealVector& x_u_bnds = natafTransform.x_upper_bounds();

      RealMatrix final_stat_grads = finalStatistics.function_gradients();
      int num_final_stats = final_stat_grads.numCols();
      for (size_t j=0; j<num_final_grad_vars; ++j) {
	size_t deriv_j = find_index(cv_ids, final_dvv[j]); //final_dvv[j]-1;
	if ( deriv_j <  numContDesVars ||
	     deriv_j >= numContDesVars+numContAleatUncVars ) {
	  // augmented design variable sensitivity
	  factor = 2. / (x_u_bnds(deriv_j) - x_l_bnds(deriv_j));
	  for (size_t i=0; i<num_final_stats; ++i)
	    final_stat_grads(j,i) *= factor; // see jacobian_dZ_dX()
	}
	// else inserted design variable sensitivity: no scaling required
      }
      finalStatistics.function_gradients(final_stat_grads);
      
      // This approach is more general, but is overkill for this purpose
      // and incurs additional copying overhead.
      /*
      RealVector initial_pt_x_pv, fn_grad_u, fn_grad_x;
      copy_data(initial_pt_x, initial_pt_x_pv);
      RealMatrix jacobian_ux;
      natafTransform.jacobian_dU_dX(initial_pt_x_pv, jacobian_ux);
      RealBaseVector final_stat_grad;
      for (i=0; i<num_final_stats; ++i) {
	copy_data(finalStatistics.function_gradient(i), fn_grad_u);
	natafTransform.trans_grad_U_to_X(fn_grad_u, fn_grad_x, jacobian_ux,
	                                 final_dvv);
	copy_data(fn_grad_x, final_stat_grad);
	finalStatistics.function_gradient(final_stat_grad, i)
      }
      */
    }

    // For distinct vars, nothing additional is needed since u_space_sampler
    // has been configured to compute dg/ds at each of the sample points.
    // uSpaceModel.build_approximation() -> PecosApproximation::build()
    // then constructs PCE/SC approximations of these gradients, and
    // PecosApproximation::get_<mean,variance>_gradient()
    // are used above to generate dmu/ds, dsigma/ds, and dbeta/ds.
  }
}


void NonDExpansion::print_covariance(std::ostream& s)
{
  s << "\nCovariance among response functions:\n";
  write_data(s, respCovariance, true, true, true);
}


void NonDExpansion::print_global_sensitivity(std::ostream& s)
{
  const StringArray& fn_labels = iteratedModel.response_labels();
  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();

  // TO DO: output for 3 settings: NO_VBD, UNIVARIATE_VBD, ALL_VBD
  // univariate = (main Si + Ti vs. x_i in rectangular table)
  // all        = (main Si + Ti vs. x_i in rectangular table,
  //               plus interation Si vs. x_ij in continued column)

  s << "\nGlobal sensitivity indices for each response function:\n";
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep
    = (PecosApproximation*)poly_approxs[0].approx_rep();
  size_t i, num_indices = poly_approx_rep->sobol_indices().length(); 
  const Pecos::IntIntMap& s_index_map = poly_approx_rep->sobol_index_map();
  Pecos::IntIntMCIter map_cit;
  StringMultiArray sobol_labels(boost::extents[num_indices]);
  // Convert numbers to binary and create labels
  for (map_cit=s_index_map.begin(); map_cit!=s_index_map.end(); ++map_cit)
    // Convert i to binary and then use binary representation to create labels
    for (int k=0; k<std::numeric_limits<int>::digits; ++k)
      if (map_cit->first & (1 << k))
	sobol_labels[map_cit->second] += cv_labels[k] + " ";
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep->expansion_coefficient_flag()) {
      // UNIVARIATE_VBD: main effects only; ALL_VBD: main+interaction effects
      s << fn_labels[i] << " Sobol indices:\n";
      //write_data(s, poly_approx_rep->sobol_indices()); // if no label array
      write_data_partial(s, 1, num_indices-1, poly_approx_rep->sobol_indices(),
			 sobol_labels);
      // total effects are always printed (UNIVARIATE_VBD or ALL_VBD)
      s << fn_labels[i] << " total Sobol indices:\n";
      write_data(s, poly_approx_rep->total_sobol_indices(), cv_labels);
    }
  }
}


/*
void NonDExpansion::print_total_effects(std::ostream& s)
{
  const StringArray& fn_labels = iteratedModel.response_labels();
  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep->expansion_coefficient_flag()) {
      s << fn_labels[i] << " total Sobol indices:\n";
      write_data(s, poly_approx_rep->total_sobol_indices(), cv_labels);
    }
  }
}
*/


void NonDExpansion::print_local_sensitivity(std::ostream& s)
{
  const StringArray& fn_labels = iteratedModel.response_labels();
  s << "\nLocal sensitivities for each response function evaluated at "
    << "uncertain variable means:\n";
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t i=0; i<numFunctions; ++i) {
    PecosApproximation* poly_approx_rep
      = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep->expansion_coefficient_flag()) {
      s << fn_labels[i] << ":\n";
      write_col_vector_trans(s, (int)i, true, true, true, expGradsMeanX);
    }
  }
}


void NonDExpansion::print_results(std::ostream& s)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);
  const StringArray& fn_labels = iteratedModel.response_labels();
  size_t i, j, cntr;

  s << "-------------------------------------------------------------------"
    << "\nStatistics derived analytically from polynomial expansion:\n";

  s << "\nMoments for each response function:\n";
  for (i=0; i<numFunctions; ++i) {
    s << fn_labels[i]
      << ":  Mean = "     << std::setw(write_precision+7) << meanStats[i]
      << "  Std. Dev. = " << std::setw(write_precision+6) << stdDevStats[i]
      << "  Coeff. of Variation = ";
    if (std::fabs(meanStats[i]) > 1.e-25)
      s << std::setw(write_precision+7) << stdDevStats[i]/meanStats[i] << '\n';
    else
      s << "Undefined\n";
  }

  if ( ( !subIteratorFlag && outputLevel >= NORMAL_OUTPUT ) ||
       (  subIteratorFlag && outputLevel >= DEBUG_OUTPUT ) ) {
    print_covariance(s);
    print_local_sensitivity(s);
  }
  if (vbdControl)
    print_global_sensitivity(s);

  if ( totalLevelRequests && ( betaMappings || expSampling ) ) {
    // match compute_distribution_mappings
    s << "\nStatistics based on ";
    if (expSampling)
      s << numSamplesOnExpansion << " samples performed on polynomial "
	<< "expansion:\n";
    else
      s << "projection of analytic moments:\n";
    NonDSampling* exp_sampler_rep
      = (NonDSampling*)expansionSampler.iterator_rep();
    exp_sampler_rep->print_distribution_mappings(s);
    // sample correlations are superceded by analytic VBD
    //if (!subIteratorFlag && expSampling)
    //  exp_sampler_rep->print_correlations(s);
    if (impSampling) {
      s << "Importance Sampling estimates:\n";
      cntr=0;
      for (i=0; i<numFunctions; i++){
        s << "Response Function " << i << "\n";
        cntr +=2;
	size_t rl_len = requestedRespLevels[i].length();
	for (j=0; j<rl_len; j++, cntr++) {
	    s << "   " << requestedRespLevels[i][j] << "   "
	      <<  finalStatistics.function_value(cntr) << "\n";
        }
      }
    }
  } 
  s << "-------------------------------------------------------------------"
    << std::endl;
}

} // namespace Dakota
