/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ProbabilityTransformModel
//- Description: Implementation code for the ProbabilityTransformModel class
//- Owner:       Brian Adams
//- Checked by:

#include "ProbabilityTransformModel.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota
{

/// initialization of static needed by RecastModel
ProbabilityTransformModel* ProbabilityTransformModel::ptmInstance(NULL);

ProbabilityTransformModel::
ProbabilityTransformModel(const Model& x_model,
                          bool truncated_bounds, Real bound) :
  RecastModel(x_model), distParamDerivs(false),
  truncatedBounds(truncated_bounds), boundVal(bound), mappingInitialized(false)
{
  ptmInstance = this;
  modelType = "probability_transform";

  numContinuousVars = subModel.cv();
  numDiscreteIntVars = subModel.div();
  numDiscreteStringVars = subModel.dsv();
  numDiscreteRealVars = subModel.drv();
  numFunctions = subModel.num_functions();

  initialize_sizes();

  initialize_random_variable_transformation();
  initialize_random_variable_types(STD_NORMAL_U);
  initialize_random_variable_correlations();

  size_t i, num_cdv_cauv = numContDesVars+numContAleatUncVars;
  Sizet2DArray vars_map, primary_resp_map, secondary_resp_map;
  SizetArray recast_vars_comps_total; // default: no change in cauv total
  // we do not reorder the u-space variable types such that we preserve a
  // 1-to-1 mapping with consistent ordering
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
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = natafTransform.x_random_variables();
  const Pecos::ShortArray& u_types = natafTransform.u_types();
  bool nonlinear_vars_map = false;
  short x_type, u_type;
  for (i=numContDesVars; i<num_cdv_cauv; ++i) {
    x_type = x_ran_vars[i].type();
    u_type = u_types[i];
    if ( x_type != u_type &&
         !( x_type == Pecos::NORMAL && u_type == Pecos::STD_NORMAL ) &&
         !( ( x_type == Pecos::UNIFORM || x_type == Pecos::HISTOGRAM_BIN ||
              x_type == Pecos::CONTINUOUS_DESIGN   ||
              x_type == Pecos::CONTINUOUS_INTERVAL ||
              x_type == Pecos::CONTINUOUS_STATE ) &&
            u_type == Pecos::STD_UNIFORM ) &&
         !( x_type == Pecos::EXPONENTIAL && u_type == Pecos::STD_EXPONENTIAL) &&
         !( x_type == Pecos::BETA   && u_type == Pecos::STD_BETA ) &&
         !( x_type == Pecos::GAMMA  && u_type == Pecos::STD_GAMMA ) ) {
      nonlinear_vars_map = true;
      break;
    }
  }

  // There is no additional response mapping beyond that required by the
  // nonlinear variables mapping.
  BoolDequeArray nonlinear_resp_map(numFunctions, BoolDeque(1, false));
  const Response& x_resp = subModel.current_response();
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  const BitArray& all_relax_di = svd.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = svd.all_relaxed_discrete_real();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!x_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!x_resp.function_hessians().empty())  recast_resp_order |= 4;

  RecastModel::
  init_sizes(recast_vars_comps_total, all_relax_di, all_relax_dr, numFunctions,
             0, 0,recast_resp_order);

  RecastModel::
  init_maps(vars_map, nonlinear_vars_map, vars_u_to_x_mapping,
	    set_u_to_x_mapping, primary_resp_map, secondary_resp_map,
            nonlinear_resp_map, resp_x_to_u_mapping, NULL);

  // publish inverse mappings for use in data imports.  Since derivatives are
  // not imported and response values are not transformed, an inverse variables
  // transformation is sufficient for this purpose.
  RecastModel::inverse_mappings(vars_x_to_u_mapping, NULL, NULL, NULL);
}


ProbabilityTransformModel::~ProbabilityTransformModel()
{ }


bool ProbabilityTransformModel::initialize_mapping(ParLevLIter pl_iter)
{
  RecastModel::initialize_mapping(pl_iter);

  bool sub_model_resize = subModel.initialize_mapping(pl_iter);

  initialize_random_variable_parameters();
  transform_correlations();

  transform_model(truncatedBounds, boundVal);

  // update message lengths for send/receive of parallel jobs (normally
  // performed once in Model::init_communicators() just after construct time)
  if (sub_model_resize)
    estimate_message_lengths();

  mappingInitialized = true;

  return sub_model_resize;
}


bool ProbabilityTransformModel::finalize_mapping()
{
  mappingInitialized = false;

  bool sub_model_resize = subModel.finalize_mapping();

  RecastModel::finalize_mapping();

  return sub_model_resize; // This will become true when TODO is implemented.
}


void ProbabilityTransformModel::initialize_sizes()
{
  // initialize sizes to zero:
  numUncertainVars = 0;
  numDesignVars = 0;
  numStateVars = 0;
  numAleatoryUncVars = 0;
  numEpistemicUncVars = 0;
  numContDesVars = 0;
  numContIntervalVars = 0;
  numContStateVars = 0;

  bool err_flag = false;
  const Variables& vars = subModel.current_variables();
  short active_view = vars.view().first;
  const SharedVariablesData& svd = vars.shared_data();

  // update sizes for aleatory uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_ALEATORY_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_ALEATORY_UNCERTAIN) { // aleatory or both
    const Pecos::AleatoryDistParams& adp
      = subModel.aleatory_distribution_parameters();
    numNormalVars       = adp.normal_means().length();
    numLognormalVars    = adp.lognormal_means().length();
    numUniformVars      = adp.uniform_lower_bounds().length();
    numLoguniformVars   = adp.loguniform_lower_bounds().length();
    numTriangularVars   = adp.triangular_modes().length();
    numExponentialVars  = adp.exponential_betas().length();
    numBetaVars         = adp.beta_alphas().length();
    numGammaVars        = adp.gamma_alphas().length();
    numGumbelVars       = adp.gumbel_alphas().length();
    numFrechetVars      = adp.frechet_alphas().length();
    numWeibullVars      = adp.weibull_alphas().length();
    numHistogramBinVars = adp.histogram_bin_pairs().size();

    numPoissonVars     = adp.poisson_lambdas().length();
    numBinomialVars    = adp.binomial_probability_per_trial().length();
    numNegBinomialVars = adp.negative_binomial_probability_per_trial().length();
    numGeometricVars   = adp.geometric_probability_per_trial().length();
    numHyperGeomVars   = adp.hypergeometric_num_drawn().length();
    numHistogramPtIntVars = adp.histogram_point_int_pairs().size();

    numHistogramPtStringVars = adp.histogram_point_string_pairs().size();

    numHistogramPtRealVars   = adp.histogram_point_real_pairs().size();

    svd.aleatory_uncertain_counts(numContAleatUncVars, numDiscIntAleatUncVars,
                                  numDiscStringAleatUncVars,
                                  numDiscRealAleatUncVars);
    numAleatoryUncVars = numContAleatUncVars       + numDiscIntAleatUncVars
                         + numDiscStringAleatUncVars + numDiscRealAleatUncVars;
  }

  // update sizes for epistemic uncertain variables
  if (active_view == RELAXED_ALL || active_view == RELAXED_UNCERTAIN ||
      active_view == RELAXED_EPISTEMIC_UNCERTAIN ||
      active_view == MIXED_ALL || active_view == MIXED_UNCERTAIN ||
      active_view == MIXED_EPISTEMIC_UNCERTAIN) { // epistemic or both
    const Pecos::EpistemicDistParams& edp
      = subModel.epistemic_distribution_parameters();
    numContIntervalVars  = edp.continuous_interval_basic_probabilities().size();

    numDiscIntervalVars  = edp.discrete_interval_basic_probabilities().size();
    numDiscSetIntUncVars = edp.discrete_set_int_values_probabilities().size();

    numDiscSetStringUncVars
      = edp.discrete_set_string_values_probabilities().size();

    numDiscSetRealUncVars = edp.discrete_set_real_values_probabilities().size();

    svd.epistemic_uncertain_counts(numContEpistUncVars, numDiscIntEpistUncVars,
                                   numDiscStringEpistUncVars,
                                   numDiscRealEpistUncVars);
    numEpistemicUncVars = numContEpistUncVars       + numDiscIntEpistUncVars
                          + numDiscStringEpistUncVars + numDiscRealEpistUncVars;
  }

  // default mode definition (can be overridden in derived classes, e.g.,
  // based on NonDSampling::samplingVarsMode):
  epistemicStats = (numEpistemicUncVars > 0);

  // update total uncertain variables
  numUncertainVars = numAleatoryUncVars + numEpistemicUncVars;

  // update sizes for design variables (if active)
  if (active_view == RELAXED_ALL    || active_view == MIXED_ALL ||
      active_view == RELAXED_DESIGN || active_view == MIXED_DESIGN) {
    svd.design_counts(numContDesVars, numDiscIntDesVars, numDiscStringDesVars,
                      numDiscRealDesVars);
    numDesignVars = numContDesVars       + numDiscIntDesVars
                    + numDiscStringDesVars + numDiscRealDesVars;
  }

  // update sizes for state variables (if active)
  if (active_view == RELAXED_ALL   || active_view == MIXED_ALL ||
      active_view == RELAXED_STATE || active_view == MIXED_STATE) {
    svd.state_counts(numContStateVars, numDiscIntStateVars,
                     numDiscStringStateVars, numDiscRealStateVars);
    numStateVars = numContStateVars       + numDiscIntStateVars
                   + numDiscStringStateVars + numDiscRealStateVars;
  }

  if ( !numUncertainVars && !numDesignVars && !numStateVars ) {
    Cerr << "\nError: number of active variables must be nonzero in Dakota::"
         << "ProbabilityTransformModel::initialize_sizes()." << std::endl;
    err_flag = true;
  }
  if (numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
      numDiscreteRealVars != numDesignVars + numUncertainVars + numStateVars) {
    Cout << "\nError: inconsistent active variable counts ("
         << numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
         numDiscreteRealVars << ", " << numDesignVars + numUncertainVars +
         numStateVars << ") in Dakota::ProbabilityTransformModel::initialize_sizes()."
         << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);
}

void ProbabilityTransformModel::transform_model(bool truncated_bounds, Real bound)
{
  ///////////////////////
  // Perform recasting //
  ///////////////////////

  size_t i, num_cdv_cauv = numContDesVars+numContAleatUncVars;
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = natafTransform.x_random_variables();
  const Pecos::ShortArray& u_types = natafTransform.u_types();
  short x_type, u_type;

  // Update continuous aleatory variable types (needed for Model::
  // continuous_{probability_density,distribution_bounds,distribution_moment}())
  for (i=numContDesVars; i<num_cdv_cauv; ++i)
    continuous_variable_type(pecos_to_dakota_variable_type(u_types[i]), i);

  /////////////////////////////////////////////////////////////////
  // Populate aleatory distribution params for transformed space //
  /////////////////////////////////////////////////////////////////

  // *** Note ***: For use with REGRESSION approaches, variable ordering in
  // get_parameter_sets() does not use x_types/u_types as in ProbabilityTransformModelQuadrature/
  // ProbabilityTransformModelSparseGrid and thus a possibility for future ordering errors exists.
  // Currently, Design/State -> Uniform is handled separately via global bounds
  // and no current x->u selection in ProbabilityTransformModel::initialize_random_variable_types()
  // causes an ordering discrepancy.  If a future mapping causes an ordering
  // inconsistency, this could be handled above via the RecastModel vars_map.
  size_t num_u_nuv = 0, num_u_bnuv = 0, num_u_lnuv = 0, num_u_blnuv = 0,
         num_u_uuv = 0, num_u_luuv = 0, num_u_tuv = 0, num_u_euv = 0,
         num_u_buv = 0, num_u_gauv = 0, num_u_guuv = 0, num_u_fuv = 0,
         num_u_wuv = 0, num_u_hbuv = 0;
  for (i=numContDesVars; i<num_cdv_cauv; ++i)
    switch (u_types[i]) {
    case Pecos::STD_NORMAL:
      ++num_u_nuv;
      break;
    case Pecos::BOUNDED_NORMAL:
      ++num_u_bnuv;
      break;
    case Pecos::LOGNORMAL:
      ++num_u_lnuv;
      break;
    case Pecos::BOUNDED_LOGNORMAL:
      ++num_u_blnuv;
      break;
    case Pecos::STD_UNIFORM:
      ++num_u_uuv;
      break;
    case Pecos::LOGUNIFORM:
      ++num_u_luuv;
      break;
    case Pecos::TRIANGULAR:
      ++num_u_tuv;
      break;
    case Pecos::STD_EXPONENTIAL:
      ++num_u_euv;
      break;
    case Pecos::STD_BETA:
      ++num_u_buv;
      break;
    case Pecos::STD_GAMMA:
      ++num_u_gauv;
      break;
    case Pecos::GUMBEL:
      ++num_u_guuv;
      break;
    case Pecos::FRECHET:
      ++num_u_fuv;
      break;
    case Pecos::WEIBULL:
      ++num_u_wuv;
      break;
    case Pecos::HISTOGRAM_BIN:
      ++num_u_hbuv;
      break;
    }
  const Pecos::AleatoryDistParams& x_adp
    = subModel.aleatory_distribution_parameters();
  Pecos::AleatoryDistParams&       u_adp
    = aleatory_distribution_parameters();
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  if (num_u_nuv || num_u_bnuv) {
    size_t num_total_nuv = num_u_nuv + num_u_bnuv;
    // u_adp was lightweight constructed; must size before entry assign
    u_adp.nuv(num_total_nuv);
    if (num_u_bnuv) {
      size_t u_n_cntr = 0, x_n_cntr = 0;
      for (i=numContDesVars; i<num_cdv_cauv; ++i) {
        x_type = x_ran_vars[i].type();
        u_type = u_types[i];
        if (u_type == Pecos::BOUNDED_NORMAL) {
          u_adp.normal_mean(x_adp.normal_mean(x_n_cntr), u_n_cntr);
          u_adp.normal_std_deviation(
            x_adp.normal_std_deviation(x_n_cntr), u_n_cntr);
          u_adp.normal_lower_bound(x_adp.normal_lower_bound(x_n_cntr),u_n_cntr);
          u_adp.normal_upper_bound(x_adp.normal_upper_bound(x_n_cntr),u_n_cntr);
          ++u_n_cntr;
        } else if (u_type == Pecos::STD_NORMAL) {
          u_adp.normal_mean(0., u_n_cntr);
          u_adp.normal_std_deviation(1., u_n_cntr);
          u_adp.normal_lower_bound(-dbl_inf, u_n_cntr);
          u_adp.normal_upper_bound( dbl_inf, u_n_cntr);
          ++u_n_cntr;
        }
        if (x_type == Pecos::NORMAL || x_type == Pecos::BOUNDED_NORMAL)
          ++x_n_cntr;
      }
    } else {
      for (i=0; i<num_total_nuv; ++i) {
        u_adp.normal_mean(0., i);
        u_adp.normal_std_deviation(1., i);
        u_adp.normal_lower_bound(-dbl_inf, i);
        u_adp.normal_upper_bound( dbl_inf, i);
      }
    }
  }
  if (num_u_lnuv || num_u_blnuv) {
    u_adp.lognormal_means(x_adp.lognormal_means());
    u_adp.lognormal_std_deviations(x_adp.lognormal_std_deviations());
    u_adp.lognormal_lambdas(x_adp.lognormal_lambdas());
    u_adp.lognormal_zetas(x_adp.lognormal_zetas());
    u_adp.lognormal_error_factors(x_adp.lognormal_error_factors());
    if (num_u_blnuv) {
      u_adp.lognormal_lower_bounds(x_adp.lognormal_lower_bounds());
      u_adp.lognormal_upper_bounds(x_adp.lognormal_upper_bounds());
    }
  }
  if (num_u_uuv) {
    u_adp.uuv(num_u_uuv); // size before entry assign
    for (i=0; i<num_u_uuv; ++i) {
      u_adp.uniform_lower_bound(-1., i);
      u_adp.uniform_upper_bound(1., i);
    }
  }
  if (num_u_luuv) {
    u_adp.loguniform_lower_bounds(x_adp.loguniform_lower_bounds());
    u_adp.loguniform_upper_bounds(x_adp.loguniform_upper_bounds());
  }
  if (num_u_tuv) {
    u_adp.triangular_modes(x_adp.triangular_modes());
    u_adp.triangular_lower_bounds(x_adp.triangular_lower_bounds());
    u_adp.triangular_upper_bounds(x_adp.triangular_upper_bounds());
  }
  if (num_u_euv) {
    u_adp.euv(num_u_euv); // size before entry assign
    for (i=0; i<num_u_euv; ++i)
      u_adp.exponential_beta(1., i);
  }
  if (num_u_buv) {
    u_adp.buv(num_u_buv); // size before entry assign
    for (i=0; i<num_u_buv; ++i) {
      u_adp.beta_alpha(x_adp.beta_alpha(i), i);
      u_adp.beta_beta(x_adp.beta_beta(i), i);
      u_adp.beta_lower_bound(-1., i);
      u_adp.beta_upper_bound( 1., i);
    }
  }
  if (num_u_gauv) {
    u_adp.gauv(num_u_gauv); // size before entry assign
    for (i=0; i<num_u_gauv; ++i) {
      u_adp.gamma_alpha(x_adp.gamma_alpha(i), i);
      u_adp.gamma_beta(1., i);
    }
  }
  if (num_u_guuv) {
    u_adp.gumbel_alphas(x_adp.gumbel_alphas());
    u_adp.gumbel_betas(x_adp.gumbel_betas());
  }
  if (num_u_fuv) {
    u_adp.frechet_alphas(x_adp.frechet_alphas());
    u_adp.frechet_betas(x_adp.frechet_betas());
  }
  if (num_u_wuv) {
    u_adp.weibull_alphas(x_adp.weibull_alphas());
    u_adp.weibull_betas(x_adp.weibull_betas());
  }
  if (num_u_hbuv)
    u_adp.histogram_bin_pairs(x_adp.histogram_bin_pairs());

  //////////////////////////////////////////////////////////////////
  // Populate epistemic distribution params for transformed space //
  //////////////////////////////////////////////////////////////////

  const Pecos::EpistemicDistParams& x_edp
    = subModel.epistemic_distribution_parameters();
  size_t num_ciuv = x_edp.ceuv();
  if (num_ciuv) {
    RealRealPairRealMapArray ciuv_bpa(num_ciuv);
    RealRealPair bnds(-1., 1.);
    for (i=0; i<num_ciuv; ++i) // one standard cell per variable in u-space
      ciuv_bpa[i][bnds] = 1.;
    Pecos::EpistemicDistParams& u_edp
      = epistemic_distribution_parameters();
    u_edp.continuous_interval_basic_probabilities(ciuv_bpa);
  }

  /////////////////////////////////////////////////
  // Populate model bounds for transformed space //
  /////////////////////////////////////////////////

  // [-1,1] are standard bounds for design, state, epistemic, uniform, & beta
  RealVector c_l_bnds(numContinuousVars, false);
  c_l_bnds = -1.;
  RealVector c_u_bnds(numContinuousVars, false);
  c_u_bnds =  1.;
  if (truncated_bounds) {
    // truncate unbounded distributions for approaches requiring bounds:
    //   standard sampling modes: model bounds only used for design/state
    //   *_UNIFORM modes: model bounds are used for all active variables
    size_t nuv_cntr = 0, lnuv_cntr = 0, gauv_cntr = 0, guuv_cntr = 0,
           fuv_cntr = 0, wuv_cntr = 0;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL:      // mean +/- bound std devs
        c_l_bnds[i] = -bound;
        c_u_bnds[i] =    bound;
        break;
      case Pecos::STD_EXPONENTIAL: // [0, mean + bound std devs] for beta=1
        c_l_bnds[i] =     0.;
        c_u_bnds[i] = 1.+bound;
        break;
      case Pecos::STD_GAMMA: {
        Real mean, stdev;
        Pecos::GammaRandomVariable::
        moments_from_params(x_adp.gamma_alpha(gauv_cntr), 1., mean, stdev);
        c_l_bnds[i] = 0.;
        c_u_bnds[i] = mean + bound * stdev;
        break;
      }
      case Pecos::BOUNDED_NORMAL: {
        // Note: as for NIDR initialization, we use the gauss{Mean,StdDev}
        // parameters rather than computing the actual mean,std_dev of the
        // bounded distribution
        Real l_bnd = x_adp.normal_lower_bound(nuv_cntr),
             u_bnd = x_adp.normal_upper_bound(nuv_cntr);
        c_l_bnds[i] = (l_bnd > -dbl_inf) ? l_bnd : // use specified bound
                      x_adp.normal_mean(nuv_cntr)              // infer bound
                      - bound * x_adp.normal_std_deviation(nuv_cntr);
        c_u_bnds[i] = (u_bnd <  dbl_inf) ? u_bnd : // use specified bound
                      x_adp.normal_mean(nuv_cntr)              // infer bound
                      + bound * x_adp.normal_std_deviation(nuv_cntr);
        break;
      }
      case Pecos::BOUNDED_LOGNORMAL: {
        c_l_bnds[i] = x_adp.lognormal_lower_bound(lnuv_cntr); // specified or 0
        Real u_bnd  = x_adp.lognormal_upper_bound(lnuv_cntr);
        if (u_bnd < dbl_inf)
          c_u_bnds[i] = u_bnd; // use specified bound
        else {                 // infer bound
          // Note: as for NIDR initialization, we use the mean,std_dev
          // parameters rather than computing the actual mean,std_dev of the
          // bounded distribution
          Real mean, stdev;
          Pecos::moments_from_lognormal_spec(x_adp.lognormal_means(),
                                             x_adp.lognormal_std_deviations(), x_adp.lognormal_lambdas(),
                                             x_adp.lognormal_zetas(), x_adp.lognormal_error_factors(),
                                             lnuv_cntr, mean, stdev);
          c_u_bnds[i] = mean + bound * stdev;
        }
        break;
      }
      case Pecos::LOGUNIFORM:
      case Pecos::TRIANGULAR:
      case Pecos::HISTOGRAM_BIN:
        // bounded distributions: x-space has desired bounds
        c_l_bnds[i] = subModel.continuous_lower_bound(i);
        c_u_bnds[i] = subModel.continuous_upper_bound(i);
        break;
      // Note: Could use subModel bounds for the following cases as well except
      // that NIDR uses +/-3 sigma, whereas here we're using +/-10 sigma
      case Pecos::LOGNORMAL: { // semi-bounded distribution
        Real mean, stdev;
        Pecos::moments_from_lognormal_spec(x_adp.lognormal_means(),
                                           x_adp.lognormal_std_deviations(), x_adp.lognormal_lambdas(),
                                           x_adp.lognormal_zetas(), x_adp.lognormal_error_factors(),
                                           lnuv_cntr, mean, stdev);
        c_l_bnds[i] = 0.;
        c_u_bnds[i] = mean + bound * stdev;
        break;
      }
      case Pecos::GUMBEL: { // unbounded distribution
        Real mean, stdev;
        Pecos::GumbelRandomVariable::
        moments_from_params(x_adp.gumbel_alpha(guuv_cntr),
                            x_adp.gumbel_beta(guuv_cntr), mean, stdev);
        c_l_bnds[i] = mean - bound * stdev;
        c_u_bnds[i] = mean + bound * stdev;
        break;
      }
      case Pecos::FRECHET: { // semibounded distribution
        Real mean, stdev;
        Pecos::FrechetRandomVariable::
        moments_from_params(x_adp.frechet_alpha(fuv_cntr),
                            x_adp.frechet_beta(fuv_cntr), mean, stdev);
        c_l_bnds[i] = 0.;
        c_u_bnds[i] = mean + bound * stdev;
        break;
      }
      case Pecos::WEIBULL: { // semibounded distribution
        Real mean, stdev;
        Pecos::WeibullRandomVariable::
        moments_from_params(x_adp.weibull_alpha(wuv_cntr),
                            x_adp.weibull_beta(wuv_cntr), mean, stdev);
        c_l_bnds[i] = 0.;
        c_u_bnds[i] = mean + bound * stdev;
        break;
      }
      }
      switch (x_ran_vars[i].type()) {
      case Pecos::NORMAL:
      case Pecos::BOUNDED_NORMAL:
        ++nuv_cntr;
        break;
      case Pecos::LOGNORMAL:
      case Pecos::BOUNDED_LOGNORMAL:
        ++lnuv_cntr;
        break;
      case Pecos::GAMMA:
        ++gauv_cntr;
        break;
      case Pecos::GUMBEL:
        ++guuv_cntr;
        break;
      case Pecos::FRECHET:
        ++fuv_cntr;
        break;
      case Pecos::WEIBULL:
        ++wuv_cntr;
        break;
      }
    }
  } else { // retain infinite model bounds where distributions are unbounded
    size_t nuv_cntr = 0, lnuv_cntr = 0;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL:
      case Pecos::GUMBEL: // unbounded distributions
        c_l_bnds[i] = -dbl_inf;
        c_u_bnds[i] = dbl_inf;
        break;
      case Pecos::LOGNORMAL:
      case Pecos::STD_EXPONENTIAL:
      case Pecos::STD_GAMMA:
      case Pecos::FRECHET:
      case Pecos::WEIBULL:                      // semibounded distributions
        c_l_bnds[i] = 0.;
        c_u_bnds[i] = dbl_inf;
        break;
      case Pecos::BOUNDED_NORMAL:
        // can't rely on subModel bounds since could be 1-sided
        c_l_bnds[i] = x_adp.normal_lower_bound(nuv_cntr);
        c_u_bnds[i] = x_adp.normal_upper_bound(nuv_cntr);
        break;
      case Pecos::BOUNDED_LOGNORMAL:
        // can't rely on subModel bounds since could be 1-sided
        c_l_bnds[i] = x_adp.lognormal_lower_bound(lnuv_cntr);
        c_u_bnds[i] = x_adp.lognormal_upper_bound(lnuv_cntr);
        break;
      case Pecos::LOGUNIFORM:
      case Pecos::TRIANGULAR:
      case Pecos::HISTOGRAM_BIN:                    // bounded distributions
        // 2-sided: can rely on subModel bounds
        c_l_bnds[i] = subModel.continuous_lower_bound(i);
        c_u_bnds[i] = subModel.continuous_upper_bound(i);
        break;
      }
      switch (x_ran_vars[i].type()) {
      case Pecos::NORMAL:
      case Pecos::BOUNDED_NORMAL:
        ++nuv_cntr;
        break;
      case Pecos::LOGNORMAL:
      case Pecos::BOUNDED_LOGNORMAL:
        ++lnuv_cntr;
        break;
      }
    }
  }
  continuous_lower_bounds(c_l_bnds);
  continuous_upper_bounds(c_u_bnds);
}

/** This function is commonly used to publish tranformation data when
    the Model variables are in a transformed space (e.g., u-space) and
    ProbabilityTransformation::ranVarTypes et al. may not be generated
    directly.  This allows for the use of inverse transformations to
    return the transformed space variables to their original states. */
void ProbabilityTransformModel::
initialize_random_variables(const Pecos::ProbabilityTransformation& transform,
                            bool deep_copy)
{
  if (deep_copy) {
    initialize_random_variable_transformation();
    natafTransform.copy(transform);
    // TO DO: deep copy of randomVarsX not yet implemented in
    // Pecos::ProbabilityTransformation::copy()
  } else
    natafTransform = transform; // shared rep

  // infer numCont{Des,Interval,State}Vars, but don't update continuous
  // aleatory uncertain counts (these may be u-space counts).
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = natafTransform.x_random_variables();
  numContDesVars = numContIntervalVars = numContStateVars = 0;
  size_t i, num_v = x_ran_vars.size();
  short x_type;
  for (i=0; i<num_v; ++i) {
    x_type = x_ran_vars[i].type();
    if      (x_type == Pecos::CONTINUOUS_DESIGN)   ++numContDesVars;
    else if (x_type == Pecos::CONTINUOUS_INTERVAL) ++numContIntervalVars;
    else if (x_type == Pecos::CONTINUOUS_STATE)    ++numContStateVars;
  }
  numContEpistUncVars = numContIntervalVars;
}


void ProbabilityTransformModel::initialize_random_variable_transformation()
{
  if (natafTransform.is_null())
    natafTransform = Pecos::ProbabilityTransformation("nataf"); // for now
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void ProbabilityTransformModel::initialize_random_variable_types(
  short u_space_type)
{
  // u_space_type is an enumeration for type of u-space transformation:
  // > if STD_NORMAL_U (reliability, AIS, and Wiener PCE/SC), then u-space is
  //   defined exclusively with std normals;
  // > if STD_UNIFORM_U (SC with local & global Hermite basis polynomials),
  //   then u-space is defined exclusively with std uniforms.
  // > if ASKEY_U (PCE/SC using Askey polynomials), then u-space is defined by
  //   std normals, std uniforms, std exponentials, std betas, and std gammas
  // > if EXTENDED_U (PCE/SC with Askey plus numerically-generated polynomials),
  //   then u-space involves at most linear scaling to std distributions.

  size_t i, av_cntr = 0, num_active_vars = subModel.cv() +
                      subModel.div() + subModel.dsv() + subModel.drv();
  ShortArray x_types(num_active_vars), u_types(num_active_vars);
  bool err_flag = false;

  for (i=0; i<numContDesVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::CONTINUOUS_DESIGN;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }
  Pecos::AleatoryDistParams& adp
    = subModel.aleatory_distribution_parameters();
  const RealVector& n_l_bnds = adp.normal_lower_bounds();
  const RealVector& n_u_bnds = adp.normal_upper_bounds();
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  for (i=0; i<numNormalVars; ++i, ++av_cntr) {
    if (n_l_bnds[i] > -dbl_inf || n_u_bnds[i] < dbl_inf) {
      x_types[av_cntr] = Pecos::BOUNDED_NORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U:
      case ASKEY_U:
        u_types[av_cntr] = Pecos::STD_NORMAL;
        break;
      case STD_UNIFORM_U:
        u_types[av_cntr] = Pecos::STD_UNIFORM;
        break;
      case EXTENDED_U:
        u_types[av_cntr] = Pecos::BOUNDED_NORMAL;
        break;
      }
    } else {
      x_types[av_cntr] = Pecos::NORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U:
      case ASKEY_U:
      case EXTENDED_U:
        u_types[av_cntr] = Pecos::STD_NORMAL;
        break;
      case STD_UNIFORM_U:
        u_types[av_cntr] = Pecos::STD_UNIFORM;
        break;
      }
    }
  }
  const RealVector& ln_l_bnds = adp.lognormal_lower_bounds();
  const RealVector& ln_u_bnds = adp.lognormal_upper_bounds();
  for (i=0; i<numLognormalVars; ++i, ++av_cntr) {
    if (ln_l_bnds[i] > 0. || ln_u_bnds[i] < dbl_inf) {
      x_types[av_cntr] = Pecos::BOUNDED_LOGNORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U:
      case ASKEY_U:
        u_types[av_cntr] = Pecos::STD_NORMAL;
        break;
      case STD_UNIFORM_U:
        u_types[av_cntr] = Pecos::STD_UNIFORM;
        break;
      case EXTENDED_U:
        u_types[av_cntr] = Pecos::BOUNDED_LOGNORMAL;
        break;
      }
    } else {
      x_types[av_cntr] = Pecos::LOGNORMAL;
      switch (u_space_type) {
      case STD_NORMAL_U:
      case ASKEY_U:
        u_types[av_cntr] = Pecos::STD_NORMAL;
        break;
      case STD_UNIFORM_U:
        u_types[av_cntr] = Pecos::STD_UNIFORM;
        break;
      case EXTENDED_U:
        u_types[av_cntr] = Pecos::LOGNORMAL;
        break;
      }
    }
  }
  for (i=0; i<numUniformVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::UNIFORM;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case ASKEY_U:
    case EXTENDED_U:
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    }
  }
  for (i=0; i<numLoguniformVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::LOGUNIFORM;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case ASKEY_U:
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::LOGUNIFORM;
      break;
    }
  }
  for (i=0; i<numTriangularVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::TRIANGULAR;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case ASKEY_U:
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::TRIANGULAR;
      break;
    }
  }
  for (i=0; i<numExponentialVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::EXPONENTIAL;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::STD_EXPONENTIAL;
      break;
    }
  }
  for (i=0; i<numBetaVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::BETA;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::STD_BETA;
      break;
    }
  }
  for (i=0; i<numGammaVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::GAMMA;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::STD_GAMMA;
      break;
    }
  }
  for (i=0; i<numGumbelVars; ++i, ++av_cntr) { // 2-sided distribution
    x_types[av_cntr] = Pecos::GUMBEL;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::GUMBEL;
      break;
    }
  }
  for (i=0; i<numFrechetVars; ++i, ++av_cntr) { // 1-sided distribution
    x_types[av_cntr] = Pecos::FRECHET;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::FRECHET;
      break;
    }
  }
  for (i=0; i<numWeibullVars; ++i, ++av_cntr) { // 1-sided distribution
    x_types[av_cntr] = Pecos::WEIBULL;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case ASKEY_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::WEIBULL;
      break;
    }
  }
  for (i=0; i<numHistogramBinVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::HISTOGRAM_BIN;
    switch (u_space_type) {
    case STD_NORMAL_U:
      u_types[av_cntr] = Pecos::STD_NORMAL;
      break;
    case ASKEY_U:
    case STD_UNIFORM_U:
      u_types[av_cntr] = Pecos::STD_UNIFORM;
      break;
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::HISTOGRAM_BIN;
      break;
    }
  }

  // discrete int aleatory uncertain
  for (i=0; i<numPoissonVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::POISSON;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case STD_UNIFORM_U:
      err_flag = true;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::POISSON;
      break;
    }
  }
  for (i=0; i<numBinomialVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::BINOMIAL;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case STD_UNIFORM_U:
      err_flag = true;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::BINOMIAL;
      break;
    }
  }
  for (i=0; i<numNegBinomialVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case STD_UNIFORM_U:
      err_flag = true;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL;
      break;
    }
  }
  
  for (i=0; i<numGeometricVars; ++i, ++av_cntr){
    x_types[av_cntr] = u_types[av_cntr] = Pecos::GEOMETRIC;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case STD_UNIFORM_U:
      err_flag = true;
      break;
    case ASKEY_U:
    case EXTENDED_U:
     u_types[av_cntr] = Pecos::GEOMETRIC;
     break;
    }
  }

  
  for (i=0; i<numHyperGeomVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::HYPERGEOMETRIC;
    switch (u_space_type) {
    case STD_NORMAL_U:
    case STD_UNIFORM_U:
      err_flag = true;
      break;
    case ASKEY_U:
    case EXTENDED_U:
      u_types[av_cntr] = Pecos::HYPERGEOMETRIC;
      break;
    }
  }
  /*
  for (i=0; i<numHistogramPtIntVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_INT;

  // discrete string aleatory uncertain
  for (i=0; i<numHistogramPtStringVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_STRING;

  // discrete real aleatory uncertain
  for (i=0; i<numHistogramPtRealVars; ++i, ++av_cntr)
    x_types[av_cntr] = u_types[av_cntr] = Pecos::HISTOGRAM_PT_REAL;
  */

  // continuous epistemic uncertain
  for (i=0; i<numContIntervalVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::CONTINUOUS_INTERVAL;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }

  // discrete int epistemic uncertain

  // discrete string epistemic uncertain

  // discrete real epistemic uncertain

  for (i=0; i<numContStateVars; ++i, ++av_cntr) {
    x_types[av_cntr] = Pecos::CONTINUOUS_STATE;
    u_types[av_cntr] = Pecos::STD_UNIFORM; // STD_NORMAL not supported
  }

  if (err_flag) {
    Cerr << "Error: unsupported mapping in ProbabilityTransformModel::"
         << "initialize_random_variable_types()." << std::endl;
    abort_handler(-1);
  } else
    natafTransform.initialize_random_variable_types(x_types, u_types);
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void ProbabilityTransformModel::initialize_random_variable_types()
{
  size_t i, av_cntr = 0, num_active_vars = subModel.cv();
  ShortArray x_types(num_active_vars);
  Pecos::AleatoryDistParams& adp
    = subModel.aleatory_distribution_parameters();
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  for (i=0; i<numContDesVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::CONTINUOUS_DESIGN;

  const RealVector& n_l_bnds = adp.normal_lower_bounds();
  const RealVector& n_u_bnds = adp.normal_upper_bounds();
  for (i=0; i<numNormalVars; ++i, ++av_cntr)
    x_types[av_cntr] = (n_l_bnds[i] > -dbl_inf || n_u_bnds[i] < dbl_inf) ?
                       Pecos::BOUNDED_NORMAL : Pecos::NORMAL;

  const RealVector& ln_l_bnds = adp.lognormal_lower_bounds();
  const RealVector& ln_u_bnds = adp.lognormal_upper_bounds();
  for (i=0; i<numLognormalVars; ++i, ++av_cntr)
    x_types[av_cntr] = (ln_l_bnds[i] > 0. || ln_u_bnds[i] < dbl_inf) ?
                       Pecos::BOUNDED_LOGNORMAL : Pecos::LOGNORMAL;

  for (i=0; i<numUniformVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::UNIFORM;

  for (i=0; i<numLoguniformVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::LOGUNIFORM;

  for (i=0; i<numTriangularVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::TRIANGULAR;

  for (i=0; i<numExponentialVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::EXPONENTIAL;

  for (i=0; i<numBetaVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::BETA;

  for (i=0; i<numGammaVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::GAMMA;

  for (i=0; i<numGumbelVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::GUMBEL;

  for (i=0; i<numFrechetVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::FRECHET;

  for (i=0; i<numWeibullVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::WEIBULL;

  for (i=0; i<numHistogramBinVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_BIN;


  // discrete int aleatory uncertain
  for (i=0; i<numPoissonVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::POISSON;
  for (i=0; i<numBinomialVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::BINOMIAL;
  for (i=0; i<numNegBinomialVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::NEGATIVE_BINOMIAL;
  for (i=0; i<numGeometricVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::GEOMETRIC;
  for (i=0; i<numHyperGeomVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HYPERGEOMETRIC;
  /*
  for (i=0; i<numHistogramPtIntVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_PT_INT;

  // discrete string aleatory uncertain
  for (i=0; i<numHistogramPtStringVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_PT_STRING;

  // discrete real aleatory uncertain
  for (i=0; i<numHistogramPtRealVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::HISTOGRAM_PT_REAL;
  */

  // continuous epistemic uncertain
  for (i=0; i<numContIntervalVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::CONTINUOUS_INTERVAL;

  // discrete int epistemic uncertain

  // discrete string epistemic uncertain

  // discrete real epistemic uncertain

  for (i=0; i<numContStateVars; ++i, ++av_cntr)
    x_types[av_cntr] = Pecos::CONTINUOUS_STATE;

  natafTransform.initialize_random_variable_types(x_types);
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void ProbabilityTransformModel::initialize_random_variable_parameters()
{
  // Be consistent with ProbabilityTransformModel active view logic.  Active counts are sufficient
  // for now, but could formalize with active view check as in ProbabilityTransformModel ctors.
  //short active_view = subModel.current_variables().view().first;
  // Note: {Aleatory,Epistemic}DistParams handles always have reps, so
  // default constructed objects are safe to interrogate for empty arrays.
  const Pecos::AleatoryDistParams&  adp = (numAleatoryUncVars)  ?
                                          subModel.aleatory_distribution_parameters()  :
                                          Pecos::AleatoryDistParams();
  const Pecos::EpistemicDistParams& edp = (numEpistemicUncVars) ?
                                          subModel.epistemic_distribution_parameters() :
                                          Pecos::EpistemicDistParams();

  RealVector cd_l_bnds, cd_u_bnds, cs_l_bnds, cs_u_bnds;
  const RealVector& c_l_bnds = subModel.continuous_lower_bounds();
  const RealVector& c_u_bnds = subModel.continuous_upper_bounds();
  if (numContDesVars) {
    cd_l_bnds = RealVector(Teuchos::View, c_l_bnds.values(), numContDesVars);
    cd_u_bnds = RealVector(Teuchos::View, c_u_bnds.values(), numContDesVars);
  }
  if (numContStateVars) {
    size_t csv_start = numContinuousVars - numContStateVars;
    cs_l_bnds	= RealVector(Teuchos::View,
                           const_cast<Real*>(&c_l_bnds[csv_start]), numContStateVars);
    cs_u_bnds = RealVector(Teuchos::View,
                           const_cast<Real*>(&c_u_bnds[csv_start]), numContStateVars);
  }
  natafTransform.initialize_random_variable_parameters(cd_l_bnds, cd_u_bnds,
      adp, edp,
      cs_l_bnds, cs_u_bnds);
}


void ProbabilityTransformModel::initialize_random_variable_correlations()
{
  const RealSymMatrix& uncertain_corr
    = subModel.aleatory_distribution_parameters().uncertain_correlations();

  if (!uncertain_corr.empty()) {
    natafTransform.initialize_random_variable_correlations(uncertain_corr);
    if (numContDesVars || numContEpistUncVars || numContStateVars)
      // expand ProbabilityTransformation::corrMatrixX to include design,
      // epistemic, state vars.  TO DO: propagate through model recursion?
      natafTransform.reshape_correlation_matrix(numContDesVars,
          numContAleatUncVars, numContEpistUncVars+numContStateVars);
  }
}


void ProbabilityTransformModel::verify_correlation_support(short u_space_type)
{
  if (natafTransform.x_correlation()) {
    const std::vector<Pecos::RandomVariable>& x_ran_vars
      = natafTransform.x_random_variables();
    const Pecos::ShortArray& u_types   = natafTransform.u_types();
    const Pecos::RealSymMatrix& x_corr = natafTransform.x_correlation_matrix();
    size_t i, j, num_cdv_cauv = numContDesVars+numContAleatUncVars;

    // We can only decorrelate in std normal space; therefore, if a variable
    // with a u_type other than STD_NORMAL is correlated with anything, revert
    // its u_type to STD_NORMAL.
    if (u_space_type != STD_NORMAL_U) {
      for (i=numContDesVars; i<num_cdv_cauv; ++i)
        if (u_types[i] != Pecos::STD_NORMAL)
          // since we don't check all rows, check *all* columns despite symmetry
          for (j=numContDesVars; j<num_cdv_cauv; ++j)
            if (i != j && std::fabs(x_corr(i, j)) > Pecos::SMALL_NUMBER) {
              Cerr << "\nWarning: u-space type for random variable "
                   << i-numContDesVars+1 << " changed to\n         "
                   << "STD_NORMAL due to decorrelation requirements.\n";
              natafTransform.u_type(Pecos::STD_NORMAL, i);
              break; // out of inner loop
            }
    }

    // Check for correlations among variable types (bounded normal, bounded
    // lognormal, loguniform, triangular, beta, and histogram) that are not
    // supported by Der Kiureghian & Liu for correlation warping estimation
    // when transforming to std normals.
    bool err_flag = false;
    for (i=numContDesVars; i<num_cdv_cauv; ++i) {
      bool distribution_error = false;
      short x_type = x_ran_vars[i].type();
      if ( x_type == Pecos::BOUNDED_NORMAL    || x_type == Pecos::LOGUNIFORM ||
           x_type == Pecos::BOUNDED_LOGNORMAL || x_type == Pecos::TRIANGULAR ||
           x_type == Pecos::BETA || x_type == Pecos::HISTOGRAM_BIN )
        // since we don't check all rows, check *all* columns despite symmetry
        for (j=numContDesVars; j<num_cdv_cauv; ++j)
          if (i != j && std::fabs(x_corr(i, j)) > Pecos::SMALL_NUMBER) {
            distribution_error = true;
            break;
          }
      if (distribution_error) {
        Cerr << "Error: correlation warping for Nataf variable transformation "
             << "of bounded normal,\n       bounded lognormal, loguniform, "
             << "triangular, beta, and histogram bin\n       distributions is "
             << "not currently supported.  Error detected for variable " << i+1
             << "." << std::endl;
        err_flag = true;
      }
    }
    if (err_flag)
      abort_handler(-1);
  }
}

unsigned short ProbabilityTransformModel::
pecos_to_dakota_variable_type(unsigned short pecos_var_type)
{
  switch (pecos_var_type) {
  case Pecos::CONTINUOUS_DESIGN:
    return CONTINUOUS_DESIGN;
    break;
  case Pecos::STD_NORMAL:
  case Pecos::NORMAL:
  case Pecos::BOUNDED_NORMAL:
    return NORMAL_UNCERTAIN;
    break;
  case Pecos::LOGNORMAL:
  case Pecos::BOUNDED_LOGNORMAL:
    return LOGNORMAL_UNCERTAIN;
    break;
  case Pecos::STD_UNIFORM:
  case Pecos::UNIFORM:
    return UNIFORM_UNCERTAIN;
    break;
  case Pecos::LOGUNIFORM:
    return LOGUNIFORM_UNCERTAIN;
    break;
  case Pecos::TRIANGULAR:
    return TRIANGULAR_UNCERTAIN;
    break;
  case Pecos::STD_EXPONENTIAL:
  case Pecos::EXPONENTIAL:
    return EXPONENTIAL_UNCERTAIN;
    break;
  case Pecos::STD_BETA:
  case Pecos::BETA:
    return BETA_UNCERTAIN;
    break;
  case Pecos::STD_GAMMA:
  case Pecos::GAMMA:
    return GAMMA_UNCERTAIN;
    break;
  case Pecos::GUMBEL:
    return GUMBEL_UNCERTAIN;
    break;
  case Pecos::FRECHET:
    return FRECHET_UNCERTAIN;
    break;
  case Pecos::WEIBULL:
    return WEIBULL_UNCERTAIN;
    break;
  case Pecos::HISTOGRAM_BIN:
    return HISTOGRAM_BIN_UNCERTAIN;
    break;
  case Pecos::POISSON:
    return POISSON_UNCERTAIN;
    break;
  case Pecos::BINOMIAL:
    return BINOMIAL_UNCERTAIN;
    break;
  case Pecos::NEGATIVE_BINOMIAL:
    return NEGATIVE_BINOMIAL_UNCERTAIN;
    break;
  case Pecos::GEOMETRIC:
    return GEOMETRIC_UNCERTAIN;
    break;
  case Pecos::HYPERGEOMETRIC:
    return HYPERGEOMETRIC_UNCERTAIN;
    break;
  case Pecos::HISTOGRAM_PT_INT:
    return HISTOGRAM_POINT_UNCERTAIN_INT;
    break;
  case Pecos::HISTOGRAM_PT_STRING:
    return HISTOGRAM_POINT_UNCERTAIN_STRING;
    break;
  case Pecos::HISTOGRAM_PT_REAL:
    return HISTOGRAM_POINT_UNCERTAIN_REAL;
    break;
  case Pecos::CONTINUOUS_INTERVAL:
    return CONTINUOUS_INTERVAL_UNCERTAIN;
    break;
  case Pecos::CONTINUOUS_STATE:
    return CONTINUOUS_STATE;
    break;
  default:
    Cerr << "Error: unsupported Pecos distribution type in "
         << "pecos_to_dakota_variable_type()." << std::endl;
    abort_handler(-1);
    return 0;
    break;
  }
}

void ProbabilityTransformModel::
resp_x_to_u_mapping(const Variables& x_vars,     const Variables& u_vars,
                    const Response&  x_response, Response&        u_response)
{
  const RealVector&         x_cv      = x_vars.continuous_variables();
  SizetMultiArrayConstView  x_cv_ids  = x_vars.continuous_variable_ids();
  SizetMultiArrayConstView  x_acv_ids = x_vars.all_continuous_variable_ids();
  const RealVector&         x_fns     = x_response.function_values();

  // In this recasting, the inputs and outputs are mapped one-to-one, with no
  // reordering.  However, the x-space ASV may be augmented from the original
  // u-space ASV due to nonlinear mapping logic in RecastModel::asv_mapping().
  const ShortArray& u_asv = u_response.active_set_request_vector();
  const SizetArray& u_dvv = u_response.active_set_derivative_vector();
  const ShortArray& x_asv = x_response.active_set_request_vector();
  const SizetArray& x_dvv = x_response.active_set_derivative_vector();
  size_t i, j, num_fns = x_asv.size(), num_deriv_vars = x_dvv.size();
  if (u_asv.size() != num_fns) {
    Cerr << "Error: inconsistent response function definition in ProbabilityTransformModel::"
         << "resp_x_to_u_mapping().\nx-space response size =\n" << num_fns
         << "u-space response size =\n" << u_asv.size() << std::endl;
    abort_handler(-1);
  }
  if (!ptmInstance->natafTransform.x_correlation() && u_dvv != x_dvv) {
    Cerr << "Error: inconsistent derivative component definition in ProbabilityTransformModel::"
         << "resp_x_to_u_mapping().\nx-space DVV =\n" << x_dvv
         << "u-space DVV =\n" << u_dvv << std::endl;
    abort_handler(-1);
  }
  bool u_grad_flag = false, u_hess_flag = false;
  for (i=0; i<num_fns; ++i) {
    if (u_asv[i] & 2)
      u_grad_flag = true;
    if (u_asv[i] & 4)
      u_hess_flag = true;
  }

  bool map_derivs = ( (u_grad_flag || u_hess_flag) &&
                      u_dvv != u_vars.inactive_continuous_variable_ids() );
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = ptmInstance->natafTransform.x_random_variables();
  const Pecos::ShortArray& u_types = ptmInstance->natafTransform.u_types();
  size_t num_types = u_types.size();
  bool nonlinear_vars_map = false;
  short x_type, u_type;
  for (i=0; i<num_types; ++i) {
    x_type = x_ran_vars[i].type();
    u_type = u_types[i];
    if ( x_type != u_type &&
         !( x_type == Pecos::NORMAL && u_type == Pecos::STD_NORMAL ) &&
         !( ( x_type == Pecos::UNIFORM || x_type == Pecos::HISTOGRAM_BIN ||
              x_type == Pecos::CONTINUOUS_DESIGN   ||
              x_type == Pecos::CONTINUOUS_INTERVAL ||
              x_type == Pecos::CONTINUOUS_STATE ) &&
            u_type == Pecos::STD_UNIFORM ) &&
         !( x_type == Pecos::EXPONENTIAL && u_type == Pecos::STD_EXPONENTIAL) &&
         !( x_type == Pecos::BETA        && u_type == Pecos::STD_BETA ) &&
         !( x_type == Pecos::GAMMA       && u_type == Pecos::STD_GAMMA ) ) {
      nonlinear_vars_map = true;
      break;
    }
  }

  RealVector   fn_grad_x,  fn_grad_us;
  RealSymMatrix      fn_hess_us;
  RealMatrix jacobian_xu, jacobian_xs;
  RealSymMatrixArray hessian_xu;

  if (map_derivs) {
    // The following transformation data is invariant w.r.t. the response fns
    // and is computed outside of the num_fns loop
    if (ptmInstance->distParamDerivs)
      ptmInstance->natafTransform.jacobian_dX_dS(x_cv, jacobian_xs,
          x_cv_ids, x_acv_ids, ptmInstance->primaryACVarMapIndices,
          ptmInstance->secondaryACVarMapTargets);
    else {
      if (u_grad_flag || u_hess_flag)
        ptmInstance->natafTransform.jacobian_dX_dU(x_cv, jacobian_xu);
      if (u_hess_flag && nonlinear_vars_map)
        ptmInstance->natafTransform.hessian_d2X_dU2(x_cv, hessian_xu);
    }
  }

  for (i=0; i<num_fns; ++i) {
    short u_asv_val = u_asv[i]; // original request from iterator
    short x_asv_val = x_asv[i]; // mapped request for sub-model

    // map value g(x) to G(u)
    if (u_asv_val & 1) {
      if ( !(x_asv_val & 1) ) {
        Cerr << "Error: missing required sub-model data in ProbabilityTransformModel::"
             << "resp_x_to_u_mapping()" << std::endl;
        abort_handler(-1);
      }
      // no transformation: g(x) = G(u) by definition
      u_response.function_value(x_fns[i], i);
    }

    // manage data requirements for derivative transformations: if fn_grad_x
    // is needed for Hessian x-form (nonlinear I/O mapping), then x_asv has been
    // augmented to include the gradient in RecastModel::asv_mapping().
    if (x_asv_val & 2)
      fn_grad_x = x_response.function_gradient_view(i);

    // map gradient dg/dx to dG/du
    if (u_asv_val & 2) {
      if ( !(x_asv_val & 2) ) {
        Cerr << "Error: missing required gradient sub-model data in ProbabilityTransformModel::"
             << "resp_x_to_u_mapping()" << std::endl;
        abort_handler(-1);
      }
      if (map_derivs) { // perform transformation
        fn_grad_us = u_response.function_gradient_view(i);
        if (ptmInstance->distParamDerivs) // transform subset of components
          ptmInstance->natafTransform.trans_grad_X_to_S(fn_grad_x,
              fn_grad_us, jacobian_xs, x_dvv, x_cv_ids, x_acv_ids,
              ptmInstance->primaryACVarMapIndices,
              ptmInstance->secondaryACVarMapTargets);
        else   // transform subset of components
          ptmInstance->natafTransform.trans_grad_X_to_U(fn_grad_x,
              fn_grad_us, jacobian_xu, x_dvv, x_cv_ids);
      } else // no transformation: dg/dx = dG/du
        u_response.function_gradient(fn_grad_x, i);
    }

    // map Hessian d^2g/dx^2 to d^2G/du^2
    if (u_asv_val & 4) {
      if ( !(x_asv_val & 4) ||
           ( map_derivs && nonlinear_vars_map && !(x_asv_val & 2) ) ) {
        Cerr << "Error: missing required sub-model data in ProbabilityTransformModel::"
             << "resp_x_to_u_mapping()" << std::endl;
        abort_handler(-1);
      }
      const RealSymMatrix& fn_hess_x = x_response.function_hessian(i);
      if (map_derivs) { // perform transformation
        fn_hess_us = u_response.function_hessian_view(i);
        if (ptmInstance->distParamDerivs) { // transform subset of components
          Cerr << "Error: Hessians with respect to inserted variables not yet "
               << "supported." << std::endl;
          abort_handler(-1);
          //ptmInstance->natafTransform.trans_hess_X_to_S(fn_hess_x,
          //  fn_hess_us, jacobian_xs, hessian_xs, fn_grad_s, x_dvv,
          //  x_cv_ids, x_vars.all_continuous_variable_ids(),
          //  ptmInstance->primaryACVarMapIndices,
          //  ptmInstance->secondaryACVarMapTargets);
        } else // transform subset of components
          ptmInstance->natafTransform.trans_hess_X_to_U(fn_hess_x, fn_hess_us,
              jacobian_xu, hessian_xu, fn_grad_x, x_dvv, x_cv_ids);
      } else // no transformation: d^2g/dx^2 = d^2G/du^2
        u_response.function_hessian(fn_hess_x, i);
    }
  }

#ifdef DEBUG
  Cout << "\nx_response:\n" << x_response
       << "\nu_response:\n" << u_response << std::endl;
#endif
}


/** Define the DVV for x-space derivative evaluations by augmenting
    the iterator requests to account for correlations. */
void ProbabilityTransformModel::set_u_to_x_mapping(const Variables& u_vars,
    const ActiveSet& u_set,
    ActiveSet& x_set)
{
  //if (ptmInstance->distParamDerivs) {
  //}
  //else
  if (ptmInstance->natafTransform.x_correlation()) {
    const SizetArray& u_dvv = u_set.derivative_vector();
    SizetMultiArrayConstView cv_ids = u_vars.continuous_variable_ids();
    SizetMultiArrayConstView icv_ids
      = u_vars.inactive_continuous_variable_ids();
    bool std_dvv = (u_dvv == cv_ids || u_dvv == icv_ids);
    if (!std_dvv) { // partial random variable derivatives: check correlations
      SizetMultiArrayConstView acv_ids = u_vars.all_continuous_variable_ids();
      size_t i, j, num_cv = cv_ids.size(), num_acv = acv_ids.size();
      SizetArray x_dvv;
      const RealSymMatrix& corr_x
        = ptmInstance->natafTransform.x_correlation_matrix();
      for (i=0; i<num_acv; ++i) { // insert in ascending order
        size_t acv_id = acv_ids[i];
        if (contains(u_dvv, acv_id))
          x_dvv.push_back(acv_id);
        else {
          size_t cv_index = find_index(cv_ids, acv_id);
          if (cv_index != _NPOS) { // random var: check correlation
            for (j=0; j<num_cv; ++j) {
              if (cv_index != j &&
                  std::fabs(corr_x(cv_index, j)) > Pecos::SMALL_NUMBER &&
                  contains(u_dvv, cv_ids[j])) {
                x_dvv.push_back(acv_id);
                break;
              }
            }
          }
        }
      }
      x_set.derivative_vector(x_dvv);
    }
  }
}



}  // namespace Dakota
