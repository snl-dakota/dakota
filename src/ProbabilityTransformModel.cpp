/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProbabilityTransformModel.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "GammaRandomVariable.hpp"
#include "GumbelRandomVariable.hpp"
#include "FrechetRandomVariable.hpp"
#include "WeibullRandomVariable.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota
{

/// initialization of static needed by RecastModel
ProbabilityTransformModel* ProbabilityTransformModel::ptmInstance(NULL);


/** Until there is a need, restrict view changes to a separate RecastModel
    recursion so that we maintain 1-to-1 active random variables here. */
ProbabilityTransformModel::
ProbabilityTransformModel(const Model& x_model, short u_space_type,
			//const ShortShortPair& recast_vars_view,
			  bool truncate_bnds, Real bnd) :
  RecastModel(x_model),                   // minimal initialization
//RecastModel(x_model, recast_vars_view), // No: for recasts limited to view
  distParamDerivs(NO_DERIVS), truncatedBounds(truncate_bnds), boundVal(bnd)
{
  modelType = "probability_transform";
  modelId   = recast_model_id(root_model_id(), "PROBABILITY_TRANSFORM");

  // initialize current{Variables,Response}, userDefinedConstraints
  const Response& x_resp = x_model.current_response();
  SizetArray recast_vars_comps_total;  // default: no change
  BitArray all_relax_di, all_relax_dr; // default: no change
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!x_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!x_resp.function_hessians().empty())  recast_resp_order |= 4;
  bool copy_values;
  init_sizes(x_model.current_variables().view(),//recast_vars_view,
	     recast_vars_comps_total, all_relax_di, all_relax_dr, numFns, 0, 0,
	     recast_resp_order, copy_values);

  // synchronize output level and grad/Hess settings with subModel
  initialize_data_from_submodel();

  // initialize invariant portions of probability transform within mvDist
  // (requires currentVariables)
  initialize_transformation(u_space_type);

  // we do not reorder the u-space variable types such that we preserve a
  // 1-to-1 mapping with consistent ordering
  const BitArray& active_vars = mvDist.active_variables();
  size_t i, num_active_rv = (active_vars.empty()) ?
    mvDist.random_variables().size() : active_vars.count();
  Sizet2DArray vars_map(num_active_rv), primary_resp_map(numFns),
    secondary_resp_map;
  for (i=0; i<num_active_rv; ++i)
    { vars_map[i].resize(1);         vars_map[i][0] = i; }
  for (i=0; i<numFns; ++i)
    { primary_resp_map[i].resize(1); primary_resp_map[i][0] = i; }
  // There is no additional response mapping beyond that required by the
  // nonlinear variables mapping.
  BoolDequeArray nonlinear_resp_map(numFns, BoolDeque(1, false));

  // initialize Variables/Response/ActiveSet recastings (requires mvDist)
  const Pecos::MultivariateDistribution& x_dist
    = x_model.multivariate_distribution();
  init_maps(vars_map, nonlinear_variables_mapping(x_dist, mvDist),
	    vars_u_to_x_mapping, set_u_to_x_mapping, primary_resp_map,
	    secondary_resp_map, nonlinear_resp_map, resp_x_to_u_mapping, NULL);
  // publish inverse mappings for use in data imports.  Since derivatives are
  // not imported and response values are not transformed, an inverse variables
  // transformation is sufficient for this purpose.
  inverse_mappings(vars_x_to_u_mapping, NULL, NULL, NULL);
  // initialize currentVariables based on subModel initial state
  inverse_transform_variables(pSubModel->current_variables(), currentVariables);
}


ProbabilityTransformModel::~ProbabilityTransformModel()
{ }


void ProbabilityTransformModel::initialize_dakota_variable_types()
{
  // Note: ctor has called initialize_distribution_{transformation,types,
  // correlations}().  Defining the transformation is deferred until
  // Model::initialize_mapping() to allow for problem resizing.

  const SharedVariablesData& u_svd = currentVariables.shared_data();
  bool cdv, ddv, cauv, dauv, ceuv, deuv, csv, dsv;
  u_svd.active_subsets(cdv, ddv, cauv, dauv, ceuv, deuv, csv, dsv);
  size_t i, num_cdv, num_ddiv, num_ddsv, num_ddrv, num_cauv, num_dauiv,
    num_dausv, num_daurv, num_ceuv, num_deuiv, num_deusv, num_deurv,
    num_csv, num_dsiv,  num_dssv,  num_dsrv, rv_cntr = 0, cv_cntr = 0,
    div_cntr = 0, dsv_cntr = 0, drv_cntr = 0;
  u_svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  u_svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
  u_svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
  u_svd.state_counts(num_csv, num_dsiv, num_dssv, num_dsrv);
  const Pecos::ShortArray& u_types = mvDist.random_variable_types();

  // Update active continuous/discrete variable types (needed for Model::
  // continuous_{probability_density,distribution_bounds,distribution_moment}())
  if (cdv)
    for (i=0; i<num_cdv; ++i, ++rv_cntr, ++cv_cntr)
      current_variables().continuous_variable_type(
	pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), cv_cntr);
  else
    rv_cntr += num_cdv;
  if (ddv) {
    for (i=0; i<num_ddiv; ++i, ++rv_cntr, ++div_cntr)
      current_variables().discrete_int_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), div_cntr);
    for (i=0; i<num_ddsv; ++i, ++rv_cntr, ++dsv_cntr)
      current_variables().discrete_string_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), dsv_cntr);
    for (i=0; i<num_ddrv; ++i, ++rv_cntr, ++drv_cntr)
      current_variables().discrete_real_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), drv_cntr);
  }
  else
    rv_cntr += num_ddiv + num_ddsv + num_ddrv;
  if (cauv)
    for (i=0; i<num_cauv; ++i, ++rv_cntr, ++cv_cntr)
      current_variables().continuous_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), cv_cntr);
  else
    rv_cntr += num_cauv;
  if (dauv) {
    for (i=0; i<num_dauiv; ++i, ++rv_cntr, ++div_cntr)
      current_variables().discrete_int_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), div_cntr);
    for (i=0; i<num_dausv; ++i, ++rv_cntr, ++dsv_cntr)
      current_variables().discrete_string_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), dsv_cntr);
    for (i=0; i<num_daurv; ++i, ++rv_cntr, ++drv_cntr)
      current_variables().discrete_real_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), drv_cntr);
  }
  else
    rv_cntr += num_dauiv + num_dausv + num_daurv;
  if (ceuv)
    for (i=0; i<num_ceuv; ++i, ++rv_cntr, ++cv_cntr)
      current_variables().continuous_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), cv_cntr);
  else
    rv_cntr += num_ceuv;
  if (deuv) {
    for (i=0; i<num_deuiv; ++i, ++rv_cntr, ++div_cntr)
      current_variables().discrete_int_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), div_cntr);
    for (i=0; i<num_deusv; ++i, ++rv_cntr, ++dsv_cntr)
      current_variables().discrete_string_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), dsv_cntr);
    for (i=0; i<num_deurv; ++i, ++rv_cntr, ++drv_cntr)
      current_variables().discrete_real_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), drv_cntr);
  }
  else
    rv_cntr += num_deuiv + num_deusv + num_deurv;
  if (csv)
    for (i=0; i<num_csv; ++i, ++rv_cntr, ++cv_cntr)
      current_variables().continuous_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), cv_cntr);
  else
    rv_cntr += num_csv;
  if (dsv) {
    for (i=0; i<num_dsiv; ++i, ++rv_cntr, ++div_cntr)
      current_variables().discrete_int_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), div_cntr);
    for (i=0; i<num_dssv; ++i, ++rv_cntr, ++dsv_cntr)
      current_variables().discrete_string_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), dsv_cntr);
    for (i=0; i<num_dsrv; ++i, ++rv_cntr, ++drv_cntr)
      current_variables().discrete_real_variable_type(
        pecos_to_dakota_variable_type(u_types[rv_cntr], rv_cntr), drv_cntr);
  }
  //else
  //  rv_cntr += num_dsiv + num_dssv + num_dsrv;
}


void ProbabilityTransformModel::
update_model_bounds(bool truncate_bnds, Real bnd)
{
  // Here, we finesse the continuous "global" bounds for a Model (distinct from
  // distribution bounds in Pecos::MultivariateDistribution), for benefit of
  // methods that are not distribution-aware, instead operating on bounded
  // domains (e.g., PStudyDACE).  While the same concept could be extended to
  // discrete distributions with semi-infinite support, we leave that exercise
  // to be motivated by future use cases.

  ////////////////////////////////////////////////////////////
  // Populate continuous model bounds for transformed space //
  ////////////////////////////////////////////////////////////

  const Pecos::ShortArray& u_types = mvDist.random_variable_types();
  const std::vector<Pecos::RandomVariable>& x_rv
    = pSubModel->multivariate_distribution().random_variables();
  size_t num_cv = currentVariables.cv(), num_rv = u_types.size();
  // [-1,1] are u-space bounds for design, state, epistemic, uniform, & beta
  RealVector c_l_bnds(num_cv, false);  c_l_bnds = -1.;
  RealVector c_u_bnds(num_cv, false);  c_u_bnds =  1.;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  const SharedVariablesData& u_svd = currentVariables.shared_data();
  bool cdv, ddv, cauv, dauv, ceuv, deuv, csv, dsv;
  u_svd.active_subsets(cdv, ddv, cauv, dauv, ceuv, deuv, csv, dsv);
  size_t i, num_cdv, num_ddiv, num_ddsv, num_ddrv, num_cauv, num_dauiv,
    num_dausv, num_daurv, num_ceuv, num_deuiv, num_deusv, num_deurv,
    num_csv, num_dsiv,  num_dssv,  num_dsrv, cv_cntr = 0, rv_cntr = 0;
  u_svd.design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
  u_svd.aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
  u_svd.epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
  u_svd.state_counts(num_csv, num_dsiv, num_dssv, num_dsrv);

  if (truncate_bnds) {
    // truncate unbounded distributions for approaches requiring bounds:
    // > standard sampling modes: model bounds only used for design/state
    // > *_UNIFORM modes: model bounds are used for all active variables

    // all cdv are mapped to [-1,1]
    if (cdv) cv_cntr += num_cdv;
    rv_cntr += num_cdv + num_ddiv + num_ddsv + num_ddrv;

    if (cauv) {
      for (i=0; i<num_cauv; ++i, ++cv_cntr, ++rv_cntr) {
	const Pecos::RandomVariable& rv_i = x_rv[rv_cntr];
	switch (u_types[rv_cntr]) {
	case Pecos::STD_NORMAL:      // mean +/- bnd std devs
	  c_l_bnds[cv_cntr] = -bnd;  c_u_bnds[cv_cntr] =    bnd;  break;
	case Pecos::STD_EXPONENTIAL: // [0, mean + bnd std devs] for beta=1
	  c_l_bnds[cv_cntr] = 0.;    c_u_bnds[cv_cntr] = 1.+bnd;  break;
	case Pecos::STD_GAMMA: {
	  Real alpha, mean, stdev;
	  rv_i.pull_parameter(Pecos::GA_ALPHA, alpha);
	  Pecos::GammaRandomVariable::
	    moments_from_params(alpha, 1., mean, stdev);
	  c_l_bnds[cv_cntr] = 0.;
	  c_u_bnds[cv_cntr] = mean + bnd * stdev;  break;
	}
	case Pecos::BOUNDED_NORMAL: {
	  // Note: as for NIDR initialization, we use mean,std_dev parameters
	  // rather than computing actual mean,std_dev of bounded distribution
	  Real mean, stdev, l_bnd, u_bnd;
	  rv_i.pull_parameter(Pecos::N_MEAN,    mean);
	  rv_i.pull_parameter(Pecos::N_STD_DEV, stdev);
	  rv_i.pull_parameter(Pecos::N_LWR_BND, l_bnd);
	  rv_i.pull_parameter(Pecos::N_UPR_BND, u_bnd);
	  c_l_bnds[cv_cntr] = (l_bnd > -dbl_inf) ? l_bnd : // use spec bound
	    mean - bnd * stdev;  // infer bound
	  c_u_bnds[cv_cntr] = (u_bnd <  dbl_inf) ? u_bnd : // use spec bound
	    mean + bnd * stdev; // infer bound
	  break;
	}
	case Pecos::LOGNORMAL: { // semi-bounded distribution
	  Real mean, stdev;
	  rv_i.pull_parameter(Pecos::LN_MEAN, mean);
	  rv_i.pull_parameter(Pecos::LN_STD_DEV, stdev);
	  c_l_bnds[cv_cntr] = 0.;
	  c_u_bnds[cv_cntr] = mean + bnd * stdev;
	  break;
	}
	case Pecos::BOUNDED_LOGNORMAL: {
	  Real l_bnd, u_bnd;
	  rv_i.pull_parameter(Pecos::LN_LWR_BND, l_bnd);
	  rv_i.pull_parameter(Pecos::LN_UPR_BND, u_bnd);
	  c_l_bnds[cv_cntr] = l_bnd; // spec or 0
	  if (u_bnd < dbl_inf)
	    c_u_bnds[cv_cntr] = u_bnd; // use spec bound
	  else {                       // infer bound
	    Real mean, stdev;
	    rv_i.pull_parameter(Pecos::LN_MEAN,    mean);
	    rv_i.pull_parameter(Pecos::LN_STD_DEV, stdev);
	    // Note: as for NIDR initialization, we use mean,std_dev parameters
	    // rather than computing actual mean,std_dev of bounded distribution
	    c_u_bnds[cv_cntr] = mean + bnd * stdev;
	  }
	  break;
	}
	case Pecos::LOGUNIFORM: case Pecos::TRIANGULAR:
	case Pecos::HISTOGRAM_BIN:
	  // bounded distributions: x-space has desired bounds
	  c_l_bnds[cv_cntr] = ModelUtils::continuous_lower_bound(*pSubModel, cv_cntr);
	  c_u_bnds[cv_cntr] = ModelUtils::continuous_upper_bound(*pSubModel, cv_cntr);
	  break;
	// Note: Could use subModel bounds for the following cases as well
	// except NIDR uses +/-3 sigma, whereas here we're using +/-10 sigma
	case Pecos::GUMBEL: { // unbounded distribution
	  Real alpha, beta, mean, stdev;
	  rv_i.pull_parameter(Pecos::GU_ALPHA, alpha);
	  rv_i.pull_parameter(Pecos::GU_BETA,  beta);
	  Pecos::GumbelRandomVariable::
	    moments_from_params(alpha, beta, mean, stdev);
	  c_l_bnds[cv_cntr] = mean - bnd * stdev;
	  c_u_bnds[cv_cntr] = mean + bnd * stdev;
	  break;
	}
	case Pecos::FRECHET: { // semibounded distribution
	  Real alpha, beta, mean, stdev;
	  rv_i.pull_parameter(Pecos::F_ALPHA, alpha);
	  rv_i.pull_parameter(Pecos::F_BETA,  beta);
	  Pecos::FrechetRandomVariable::
	    moments_from_params(alpha, beta, mean, stdev);
	  c_l_bnds[cv_cntr] = 0.; c_u_bnds[cv_cntr] = mean + bnd * stdev; break;
	}
	case Pecos::WEIBULL: { // semibounded distribution
	  Real alpha, beta, mean, stdev;
	  rv_i.pull_parameter(Pecos::W_ALPHA, alpha);
	  rv_i.pull_parameter(Pecos::W_BETA,  beta);
	  Pecos::WeibullRandomVariable::
	    moments_from_params(alpha, beta, mean, stdev);
	  c_l_bnds[cv_cntr] = 0.; c_u_bnds[cv_cntr] = mean + bnd * stdev; break;
	}
	}
      }
    }
    else
      rv_cntr += num_cauv;
    rv_cntr += num_dauiv + num_dausv + num_daurv;

    if (ceuv) {
      for (i=0; i<num_ceuv; ++i, ++cv_cntr, ++rv_cntr) {
	const Pecos::RandomVariable& rv_i = x_rv[rv_cntr];
	switch (u_types[rv_cntr]) {
	case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN:
	  // bounded distributions: x-space has desired bounds
	  c_l_bnds[cv_cntr] = ModelUtils::continuous_lower_bound(*pSubModel, cv_cntr);
	  c_u_bnds[cv_cntr] = ModelUtils::continuous_upper_bound(*pSubModel, cv_cntr);
	  break;
	}
      }
    }
    //else
    //  rv_cntr += num_ceuv;
    //rv_cntr += num_deuiv + num_deusv + num_deurv;

    // all csv are mapped to [-1,1]
    //if (csv) cv_cntr += num_csv;
    //rv_cntr += num_csv + num_dsiv + num_dssv + num_dsrv;
  }
  else { // retain infinite model bounds where distributions are unbounded

    // all cdv are mapped to [-1,1]
    if (cdv) cv_cntr += num_cdv;
    rv_cntr += num_cdv + num_ddiv + num_ddsv + num_ddrv;

    if (cauv) {
      for (i=0; i<num_cauv; ++i, ++cv_cntr, ++rv_cntr) {
	const Pecos::RandomVariable& rv_i = x_rv[rv_cntr];
	switch (u_types[rv_cntr]) {
	case Pecos::STD_NORMAL:  case Pecos::GUMBEL: // unbounded distributions
	  c_l_bnds[cv_cntr] = -dbl_inf;  c_u_bnds[cv_cntr] = dbl_inf;   break;
	case Pecos::LOGNORMAL:  case Pecos::STD_EXPONENTIAL:
	case Pecos::STD_GAMMA:  case Pecos::FRECHET:
	case Pecos::WEIBULL:                       // semibounded distributions
	  c_l_bnds[cv_cntr] = 0.;        c_u_bnds[cv_cntr] = dbl_inf;   break;
	case Pecos::BOUNDED_NORMAL:
	  // can't rely on subModel bounds since could be 1-sided
	  rv_i.pull_parameter(Pecos::N_LWR_BND, c_l_bnds[cv_cntr]);
	  rv_i.pull_parameter(Pecos::N_UPR_BND, c_u_bnds[cv_cntr]);
	  break;
	case Pecos::BOUNDED_LOGNORMAL:
	  // can't rely on subModel bounds since could be 1-sided
	  rv_i.pull_parameter(Pecos::LN_LWR_BND, c_l_bnds[cv_cntr]);
	  rv_i.pull_parameter(Pecos::LN_UPR_BND, c_u_bnds[cv_cntr]);
	  break;
	case Pecos::LOGUNIFORM:  case Pecos::TRIANGULAR:
	case Pecos::HISTOGRAM_BIN:                     // bounded distributions
	  // 2-sided: can rely on subModel bounds
	  c_l_bnds[cv_cntr] = ModelUtils::continuous_lower_bound(*pSubModel, cv_cntr);
	  c_u_bnds[cv_cntr] = ModelUtils::continuous_upper_bound(*pSubModel, cv_cntr); break;
	}
      }
    }
    else
      rv_cntr += num_cauv;
    rv_cntr += num_dauiv + num_dausv + num_daurv;

    if (ceuv) {
      for (i=0; i<num_ceuv; ++i, ++cv_cntr, ++rv_cntr) {
	const Pecos::RandomVariable& rv_i = x_rv[rv_cntr];
	switch (u_types[rv_cntr]) {
	case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN:
	  // bounded distributions: x-space has desired bounds
	  c_l_bnds[cv_cntr] = ModelUtils::continuous_lower_bound(*pSubModel, cv_cntr);
	  c_u_bnds[cv_cntr] = ModelUtils::continuous_upper_bound(*pSubModel, cv_cntr);
	  break;
	}
      }
    }
    //else
    //  rv_cntr += num_ceuv;
    //rv_cntr += num_deuiv + num_deusv + num_deurv;

    // all csv are mapped to [-1,1]
    //if (csv) cv_cntr += num_csv;
    //rv_cntr += num_csv + num_dsiv + num_dssv + num_dsrv;
  }

  ModelUtils::continuous_lower_bounds(*this, c_l_bnds);
  ModelUtils::continuous_upper_bounds(*this, c_u_bnds);
}


/** Build ProbabilityTransformation::ranVar arrays containing the
    uncertain variable distribution types and their corresponding
    means/standard deviations.  This function is used when the Model
    variables are in x-space. */
void ProbabilityTransformModel::
initialize_distribution_types(short u_space_type,
			      const Pecos::BitArray& active_rv,
			      const Pecos::MultivariateDistribution& x_dist,
			      Pecos::MultivariateDistribution& u_dist)
{
  // u_space_type is an enumeration for type of u-space transformation:
  // > if STD_NORMAL_U (reliability, AIS, and Wiener PCE/SC), then u-space is
  //   defined exclusively with std normals;
  // > if STD_UNIFORM_U (SC with local & global Hermite basis polynomials),
  //   then u-space is defined exclusively with std uniforms;
  // > if PARTIAL_ASKEY_U (C3 with orthog polynomials), then u-space is defined
  //   by std normals and std uniforms;
  // > if ASKEY_U (PCE/SC using Askey polynomials), then u-space is defined by
  //   std normals, std uniforms, std exponentials, std betas, and std gammas;
  // > if EXTENDED_U (PCE/SC with Askey plus numerically-generated polynomials),
  //   then u-space involves at most linear scaling to std distributions.

  const Pecos::ShortArray& x_types = x_dist.random_variable_types();
  size_t i, num_rv = x_types.size();
  Pecos::ShortArray u_types(num_rv, Pecos::NO_TYPE);
  bool err_flag = false;

  bool no_mask = active_rv.empty();
  for (i=0; i<num_rv; ++i)
    if (no_mask || active_rv[i])
      switch (u_space_type) {
      case STD_NORMAL_U:  case STD_UNIFORM_U:
	switch (x_types[i]) {
	case Pecos::DISCRETE_RANGE:      case Pecos::DISCRETE_SET_INT:
	case Pecos::DISCRETE_SET_STRING: case Pecos::DISCRETE_SET_REAL:
	case Pecos::POISSON:             case Pecos::BINOMIAL:
	case Pecos::NEGATIVE_BINOMIAL:   case Pecos::GEOMETRIC:
	case Pecos::HYPERGEOMETRIC:      case Pecos::HISTOGRAM_PT_INT:
	case Pecos::HISTOGRAM_PT_STRING: case Pecos::HISTOGRAM_PT_REAL:
	case Pecos::DISCRETE_INTERVAL_UNCERTAIN:
	case Pecos::DISCRETE_UNCERTAIN_SET_INT:
	case Pecos::DISCRETE_UNCERTAIN_SET_STRING:
	case Pecos::DISCRETE_UNCERTAIN_SET_REAL:
	  err_flag = true;                                            break;
	case Pecos::CONTINUOUS_RANGE: case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN:
	  u_types[i] = Pecos::STD_UNIFORM;                            break;
	default:
	  u_types[i] = (u_space_type == STD_UNIFORM_U)
	             ? Pecos::STD_UNIFORM : Pecos::STD_NORMAL;        break;
	}
	break;
      case PARTIAL_ASKEY_U: // used for cases with limited distrib support (C3)
	switch (x_types[i]) {
	case Pecos::NORMAL:           case Pecos::BOUNDED_NORMAL:
	case Pecos::LOGNORMAL:        case Pecos::BOUNDED_LOGNORMAL:
	case Pecos::EXPONENTIAL:      case Pecos::GAMMA:
	case Pecos::GUMBEL:           case Pecos::FRECHET:
	case Pecos::WEIBULL: // unbounded or semi-bounded dist; bounded N/logN
	  u_types[i] = Pecos::STD_NORMAL;                             break;
	case Pecos::UNIFORM:          case Pecos::LOGUNIFORM:
	case Pecos::TRIANGULAR:       case Pecos::BETA:
	case Pecos::HISTOGRAM_BIN:    case Pecos::CONTINUOUS_RANGE:
	case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN: // bounded
	  u_types[i] = Pecos::STD_UNIFORM;                            break;
	// TO DO: discrete types ddv, dauv, deuv, dsv
	default:	               err_flag = true;               break;
	}
	break;
      case ASKEY_U:
	switch (x_types[i]) {
	case Pecos::NORMAL:           case Pecos::BOUNDED_NORMAL:
	case Pecos::LOGNORMAL:        case Pecos::BOUNDED_LOGNORMAL:
	case Pecos::GUMBEL:           case Pecos::FRECHET:
	case Pecos::WEIBULL:
	  u_types[i] = Pecos::STD_NORMAL;      break;
	case Pecos::UNIFORM:          case Pecos::LOGUNIFORM:
	case Pecos::TRIANGULAR:       case Pecos::HISTOGRAM_BIN:
	case Pecos::CONTINUOUS_RANGE: case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN:
	  u_types[i] = Pecos::STD_UNIFORM;     break;
	case Pecos::EXPONENTIAL: u_types[i] = Pecos::STD_EXPONENTIAL; break;
	case Pecos::BETA:        u_types[i] = Pecos::STD_BETA;        break;
	case Pecos::GAMMA:       u_types[i] = Pecos::STD_GAMMA;       break;
	// TO DO: discrete types ddv, dauv, deuv, dsv
	//case Pecos::POISSON:           case Pecos::BINOMIAL:
	//case Pecos::NEGATIVE_BINOMIAL: case Pecos::GEOMETRIC:
	//case Pecos::HYPERGEOMETRIC:
	default:                 err_flag = true;                     break;
	}
	break;
      case EXTENDED_U:
	switch (x_types[i]) {
	case Pecos::CONTINUOUS_RANGE:  case Pecos::UNIFORM:
	  u_types[i] = Pecos::STD_UNIFORM;                            break;
	case Pecos::NORMAL:      u_types[i] = Pecos::STD_NORMAL;      break;
	case Pecos::EXPONENTIAL: u_types[i] = Pecos::STD_EXPONENTIAL; break;
	case Pecos::BETA:        u_types[i] = Pecos::STD_BETA;        break;
	case Pecos::GAMMA:       u_types[i] = Pecos::STD_GAMMA;       break;
	default:                 u_types[i] = x_types[i];             break;
	}
	break;
      }
    else // inactive variables are not transformed
      u_types[i] = x_types[i];

  if (err_flag) {
    Cerr << "Error: unsupported mapping in ProbabilityTransformModel::"
         << "initialize_distribution_transformation()." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  std::shared_ptr<Pecos::MarginalsCorrDistribution> u_dist_rep =
    std::static_pointer_cast<Pecos::MarginalsCorrDistribution>
    (u_dist.multivar_dist_rep());
  u_dist_rep->initialize_types(u_types, active_rv);
}


void ProbabilityTransformModel::verify_correlation_support(short u_space_type)
{
  Pecos::MultivariateDistribution& x_dist
    = pSubModel->multivariate_distribution();
  if (x_dist.correlation()) {
    const Pecos::ShortArray&   x_types = x_dist.random_variable_types();
    const Pecos::ShortArray&   u_types = mvDist.random_variable_types();
    const Pecos::RealSymMatrix& x_corr = x_dist.correlation_matrix();
    const Pecos::BitArray& active_corr = x_dist.active_correlations();
    size_t i, j, corr_i, corr_j, num_rv = x_types.size();
    bool no_mask = active_corr.empty();

    // We can only decorrelate in std normal space; therefore, if a variable
    // with a u_type other than STD_NORMAL is correlated with anything, change
    // its u_type to STD_NORMAL.
    if (u_space_type != STD_NORMAL_U) {
      for (i=0, corr_i=0; i<num_rv; ++i)
	if (no_mask || active_corr[i]) {
	  if (u_types[i] != Pecos::STD_NORMAL)
	    // since we don't check all rows, check all columns despite symmetry
	    for (j=0, corr_j=0; j<num_rv; ++j)
	      if (no_mask || active_corr[j]) {
		if (i != j && !Pecos::is_small(x_corr(corr_i, corr_j))) {
		  Cerr << "\nWarning: u-space type for random variable " << i+1
		       << " changed to\n         STD_NORMAL due to "
		       << "decorrelation requirements.\n";
		  mvDist.random_variable_type(Pecos::STD_NORMAL, i);
		  break; // out of inner loop
		}
		++corr_j;
	      }
	  ++corr_i;
	}

      /*
      for (i=0; i<num_rv; ++i)
	if (u_types[i] != Pecos::STD_NORMAL) {
	  // since we don't check all rows, check all columns despite symmetry
	  corr_i = rv_index_to_corr_index(i);
	  if (corr_i != _NPOS)
	    for (j=0, corr_j=0; j<num_rv; ++j)
	      if (i != j) {
		corr_j = rv_index_to_corr_index(j);
		if (corr_j != _NPOS &&
		    std::abs(x_corr(corr_i, corr_j)) > Pecos::SMALL_NUMBER) {
		  Cerr << "\nWarning: u-space type for random variable " << i+1
		       << " changed to\n         STD_NORMAL due to "
		       << "decorrelation requirements.\n";
		  mvDist.random_variable_type(Pecos::STD_NORMAL, i);
		  break; // out of inner loop
		}
	      }
	}
      */
    }

    // Check for correlations among variable types (bounded normal, bounded
    // lognormal, loguniform, triangular, beta, and histogram) that are not
    // supported by Der Kiureghian & Liu for correlation warping estimation
    // when transforming to std normals.
    bool err_flag = false;
    for (i=0, corr_i=0; i<num_rv; ++i) {
      bool distribution_error = false;
      if (no_mask || active_corr[i]) {
	short x_type = x_types[i];
	if (x_type == Pecos::BOUNDED_NORMAL    || x_type == Pecos::LOGUNIFORM ||
	    x_type == Pecos::BOUNDED_LOGNORMAL || x_type == Pecos::TRIANGULAR ||
	    x_type == Pecos::BETA || x_type == Pecos::HISTOGRAM_BIN)
	  // since we don't check all rows, check all columns despite symmetry
	  for (j=0, corr_j=0; j<num_rv; ++j)
	    if (no_mask || active_corr[j]) {
	      if (i != j &&
		  !Pecos::is_small(x_corr(corr_i, corr_j)))
		{ distribution_error = true; break; }
	      ++corr_j;
	    }
	++corr_i;
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
      abort_handler(MODEL_ERROR);
  }
}


unsigned short ProbabilityTransformModel::
pecos_to_dakota_variable_type(unsigned short pecos_var_type, size_t rv_index)
{
  const SizetArray& vc_totals
    = pSubModel->current_variables().shared_data().components_totals();
  switch (pecos_var_type) {
  case Pecos::CONTINUOUS_RANGE:    // non-unique mapping
    return (rv_index < vc_totals[TOTAL_CDV]) ? // not subject to active subsets
      CONTINUOUS_DESIGN : CONTINUOUS_STATE;                    break;
  case Pecos::DISCRETE_RANGE:      // non-unique mapping
    return (rv_index < vc_totals[TOTAL_CDV]  + vc_totals[TOTAL_DDIV]) ?
      DISCRETE_DESIGN_RANGE : DISCRETE_STATE_RANGE;            break;
  case Pecos::DISCRETE_SET_INT:    // non-unique mapping
    return (rv_index < vc_totals[TOTAL_CDV]  + vc_totals[TOTAL_DDIV]) ?
      DISCRETE_DESIGN_SET_INT : DISCRETE_STATE_SET_INT;        break;
  case Pecos::DISCRETE_SET_STRING: // non-unique mapping
    return (rv_index < vc_totals[TOTAL_CDV]  + vc_totals[TOTAL_DDIV] +
	               vc_totals[TOTAL_DDSV]) ?
      DISCRETE_DESIGN_SET_STRING : DISCRETE_STATE_SET_STRING;  break;
  case Pecos::DISCRETE_SET_REAL:   // non-unique mapping
    return (rv_index < vc_totals[TOTAL_CDV]  + vc_totals[TOTAL_DDIV] +
	               vc_totals[TOTAL_DDSV] + vc_totals[TOTAL_DDRV]) ?
      DISCRETE_DESIGN_SET_REAL : DISCRETE_STATE_SET_REAL;      break;
  // continuous aleatory
  case Pecos::STD_NORMAL: case Pecos::NORMAL: case Pecos::BOUNDED_NORMAL:
    return NORMAL_UNCERTAIN;  break;
  case Pecos::LOGNORMAL: case Pecos::BOUNDED_LOGNORMAL:
    return LOGNORMAL_UNCERTAIN; break;
  case Pecos::STD_UNIFORM: case Pecos::UNIFORM:
    return UNIFORM_UNCERTAIN; break;
  case Pecos::LOGUNIFORM:
    return LOGUNIFORM_UNCERTAIN; break;
  case Pecos::TRIANGULAR:
    return TRIANGULAR_UNCERTAIN; break;
  case Pecos::STD_EXPONENTIAL: case Pecos::EXPONENTIAL:
    return EXPONENTIAL_UNCERTAIN; break;
  case Pecos::STD_BETA: case Pecos::BETA:
    return BETA_UNCERTAIN; break;
  case Pecos::STD_GAMMA: case Pecos::GAMMA:
    return GAMMA_UNCERTAIN; break;
  case Pecos::GUMBEL:
    return GUMBEL_UNCERTAIN; break;
  case Pecos::FRECHET:
    return FRECHET_UNCERTAIN; break;
  case Pecos::WEIBULL:
    return WEIBULL_UNCERTAIN; break;
  case Pecos::HISTOGRAM_BIN:
    return HISTOGRAM_BIN_UNCERTAIN; break;
  // discrete aleatory
  case Pecos::POISSON:
    return POISSON_UNCERTAIN; break;
  case Pecos::BINOMIAL:
    return BINOMIAL_UNCERTAIN; break;
  case Pecos::NEGATIVE_BINOMIAL:
    return NEGATIVE_BINOMIAL_UNCERTAIN; break;
  case Pecos::GEOMETRIC:
    return GEOMETRIC_UNCERTAIN; break;
  case Pecos::HYPERGEOMETRIC:
    return HYPERGEOMETRIC_UNCERTAIN; break;
  case Pecos::HISTOGRAM_PT_INT:
    return HISTOGRAM_POINT_UNCERTAIN_INT; break;
  case Pecos::HISTOGRAM_PT_STRING:
    return HISTOGRAM_POINT_UNCERTAIN_STRING; break;
  case Pecos::HISTOGRAM_PT_REAL:
    return HISTOGRAM_POINT_UNCERTAIN_REAL; break;
  // continuous epistemic
  case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN:
    return CONTINUOUS_INTERVAL_UNCERTAIN; break;
  // discrete epistemic
  case Pecos::DISCRETE_INTERVAL_UNCERTAIN:
    return DISCRETE_INTERVAL_UNCERTAIN; break;
  case Pecos:: DISCRETE_UNCERTAIN_SET_INT:
    return DISCRETE_UNCERTAIN_SET_INT; break;
  case Pecos::DISCRETE_UNCERTAIN_SET_STRING:
    return DISCRETE_UNCERTAIN_SET_STRING; break;
  case Pecos::DISCRETE_UNCERTAIN_SET_REAL:
    return DISCRETE_UNCERTAIN_SET_REAL; break;
  default:
    Cerr << "Error: unsupported Pecos distribution type in "
         << "pecos_to_dakota_variable_type()." << std::endl;
    abort_handler(MODEL_ERROR);  return 0;  break;
  }
}


void ProbabilityTransformModel::
resp_x_to_u_mapping(const Variables& x_vars,     const Variables& u_vars,
                    const Response&  x_response, Response&        u_response)
{
  const RealVector&         x_cv      = x_vars.continuous_variables();
  SizetMultiArrayConstView  x_cv_ids  = x_vars.continuous_variable_ids();
  SizetMultiArrayConstView  u_cv_ids  = u_vars.continuous_variable_ids();
  SizetMultiArrayConstView  x_acv_ids = x_vars.all_continuous_variable_ids();
  const RealVector&         x_fns     = x_response.function_values();

  // In this recasting, the inputs and outputs are mapped one-to-one, with no
  // reordering.  However, the x-space ASV may be augmented from the original
  // u-space ASV due to nonlinear mapping logic in RecastModel::transform_set().
  const ShortArray& u_asv = u_response.active_set_request_vector();
  const SizetArray& u_dvv = u_response.active_set_derivative_vector();
  const ShortArray& x_asv = x_response.active_set_request_vector();
  const SizetArray& x_dvv = x_response.active_set_derivative_vector();
  Pecos::MultivariateDistribution& x_dist
    = ptmInstance->pSubModel->multivariate_distribution();
  size_t i, j, num_fns = x_asv.size(), num_deriv_vars = x_dvv.size();
  if (u_asv.size() != num_fns) {
    Cerr << "Error: inconsistent response function definition in Probability"
	 << "TransformModel::resp_x_to_u_mapping().\n       x-space response "
	 << "size = " << num_fns << ", u-space response size = " << u_asv.size()
	 << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (!x_dist.correlation() && u_dvv != x_dvv) {
    Cerr << "Error: inconsistent derivative component definition in Probability"
	 << "TransformModel::resp_x_to_u_mapping().\nx-space DVV =\n" << x_dvv
         << "u-space DVV =\n" << u_dvv << std::endl;
    abort_handler(MODEL_ERROR);
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
  RealVector   fn_grad_x,  fn_grad_us;  RealSymMatrix      fn_hess_us;
  RealMatrix jacobian_xu, jacobian_xs;  RealSymMatrixArray hessian_xu;

  if (map_derivs) {
    // The following transformation data is invariant w.r.t. the response fns
    // and is computed outside of the num_fns loop
    if (ptmInstance->distParamDerivs > NO_DERIVS)
      ptmInstance->natafTransform.jacobian_dX_dS(x_cv, jacobian_xs,
	x_cv_ids, u_cv_ids, x_acv_ids, ptmInstance->primaryACVarMapIndices,
	ptmInstance->secondaryACVarMapTargets);
    else {
      if (u_grad_flag || u_hess_flag)
        ptmInstance->natafTransform.jacobian_dX_dU(x_cv, x_cv_ids, u_cv_ids,
						   jacobian_xu);
      if (u_hess_flag && ptmInstance->nonlinearVarsMapping)
        ptmInstance->natafTransform.hessian_d2X_dU2(x_cv, x_cv_ids, u_cv_ids,
						    hessian_xu);
    }
  }

  for (i=0; i<num_fns; ++i) {
    short u_asv_val = u_asv[i]; // original request from iterator
    short x_asv_val = x_asv[i]; // mapped request for sub-model

    // map value g(x) to G(u)
    if (u_asv_val & 1) {
      if ( !(x_asv_val & 1) ) {
        Cerr << "Error: missing required sub-model data in ProbabilityTransform"
	     << "Model::resp_x_to_u_mapping()" << std::endl;
        abort_handler(MODEL_ERROR);
      }
      // no transformation: g(x) = G(u) by definition
      u_response.function_value(x_fns[i], i);
    }

    // manage data requirements for derivative transformations: if fn_grad_x
    // is needed for Hessian x-form (nonlinear I/O mapping), then x_asv has been
    // augmented to include the gradient in RecastModel::transform_set().
    if (x_asv_val & 2)
      fn_grad_x = x_response.function_gradient_view(i);

    // map gradient dg/dx to dG/du
    if (u_asv_val & 2) {
      if ( !(x_asv_val & 2) ) {
        Cerr << "Error: missing required gradient sub-model data in Probability"
	     << "TransformModel::resp_x_to_u_mapping()" << std::endl;
        abort_handler(MODEL_ERROR);
      }
      if (map_derivs) { // perform transformation
        fn_grad_us = u_response.function_gradient_view(i);
        if (ptmInstance->distParamDerivs > NO_DERIVS) // transform subset
          ptmInstance->natafTransform.trans_grad_X_to_S(fn_grad_x,
            fn_grad_us, jacobian_xs, x_dvv, x_cv_ids, u_cv_ids, x_acv_ids,
            ptmInstance->primaryACVarMapIndices,
            ptmInstance->secondaryACVarMapTargets);
        else // transform subset of components
          ptmInstance->natafTransform.trans_grad_X_to_U(fn_grad_x, x_cv_ids,
            fn_grad_us, jacobian_xu, x_dvv);
      }
      else // no transformation: dg/dx = dG/du
        u_response.function_gradient(fn_grad_x, i);
    }

    // map Hessian d^2g/dx^2 to d^2G/du^2
    if (u_asv_val & 4) {
      if ( !(x_asv_val & 4) || ( map_derivs &&
	   ptmInstance->nonlinearVarsMapping && !(x_asv_val & 2) ) ) {
        Cerr << "Error: missing required sub-model data in Probability"
	     << "TransformModel::resp_x_to_u_mapping()" << std::endl;
        abort_handler(MODEL_ERROR);
      }
      const RealSymMatrix& fn_hess_x = x_response.function_hessian(i);
      if (map_derivs) { // perform transformation
        fn_hess_us = u_response.function_hessian_view(i);
        if (ptmInstance->distParamDerivs > NO_DERIVS) { // transform subset
          Cerr << "Error: Hessians with respect to inserted variables not yet "
               << "supported." << std::endl;
          abort_handler(MODEL_ERROR);
          //ptmInstance->natafTransform.trans_hess_X_to_S(fn_hess_x,
          //  fn_hess_us, jacobian_xs, hessian_xs, fn_grad_s, x_dvv,
          //  x_cv_ids, x_vars.all_continuous_variable_ids(),
          //  ptmInstance->primaryACVarMapIndices,
          //  ptmInstance->secondaryACVarMapTargets);
        }
	else // transform subset of components
          ptmInstance->natafTransform.trans_hess_X_to_U(fn_hess_x, x_cv_ids,
	    fn_hess_us, jacobian_xu, hessian_xu, fn_grad_x, x_dvv);
      }
      else // no transformation: d^2g/dx^2 = d^2G/du^2
        u_response.function_hessian(fn_hess_x, i);
    }
  }

  u_response.metadata(x_response.metadata());

#ifdef DEBUG
  Cout << "\nx_response:\n" << x_response
       << "\nu_response:\n" << u_response << std::endl;
#endif
}


/** Define the DVV for x-space derivative evaluations by augmenting
    the iterator requests to account for correlations. */
void ProbabilityTransformModel::
set_u_to_x_mapping(const Variables& u_vars, const ActiveSet& u_set,
		   ActiveSet& x_set)
{
  Pecos::MultivariateDistribution& x_dist
    = ptmInstance->pSubModel->multivariate_distribution();
  //if (ptmInstance->distParamDerivs > NO_DERIVS) {
  //}
  //else
  if (x_dist.correlation()) {
    const SizetArray& u_dvv = u_set.derivative_vector();
    bool std_dvv = (u_dvv == u_vars.continuous_variable_ids() ||
		    u_dvv == u_vars.inactive_continuous_variable_ids());
    if (!std_dvv) { // partial random variable derivatives: check correlations
      SizetArray x_dvv;
      SizetMultiArrayConstView acv_ids = u_vars.all_continuous_variable_ids();
      size_t i, j, corr_i, corr_j, acv_id_i, num_acv = acv_ids.size();
      const RealSymMatrix& corr_x = x_dist.correlation_matrix();
      for (i=0; i<num_acv; ++i) { // insert in ascending order
        acv_id_i = acv_ids[i];
        if (contains(u_dvv, acv_id_i))
          x_dvv.push_back(acv_id_i);
        else {
	  corr_i = ptmInstance->acv_index_to_corr_index(i);
	  if (corr_i != _NPOS)
	    for (j=0; j<num_acv; ++j)
	      if (j != i) {
		corr_j = ptmInstance->acv_index_to_corr_index(j);
		if (corr_j != _NPOS &&
		    !Pecos::is_small(corr_x(corr_i, corr_j)) &&
		    contains(u_dvv, acv_ids[j]))
		  { x_dvv.push_back(acv_id_i);  break; }
	      }
        }
      }
      x_set.derivative_vector(x_dvv);
    }
  }
  // else copying DVV in RecastModel::transform_set() is sufficient
}

}  // namespace Dakota
