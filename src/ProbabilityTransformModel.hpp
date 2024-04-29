/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef PROBABILITY_TRANSFORM_MODEL_H
#define PROBABILITY_TRANSFORM_MODEL_H

#include "RecastModel.hpp"
#include "ProbabilityTransformation.hpp"

namespace Dakota
{


/// Probability transformation specialization of RecastModel

/** Specialization of RecastModel to transform a sub-model to u-space. */
class ProbabilityTransformModel: public RecastModel
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  ProbabilityTransformModel(const Model& sub_model, short u_space_type,
			  //const ShortShortPair& recast_vars_view,
			    bool truncate_bnds = false, Real bnd = 10.);

  /// destructor
  ~ProbabilityTransformModel();

  //
  //- Heading: Member functions
  //

  /// initialize transformed distribution types and instantiate mvDist
  static void initialize_distribution_types(
    short u_space_type, const Pecos::BitArray& active_rv,
    const Pecos::MultivariateDistribution& x_dist,
    Pecos::MultivariateDistribution& u_dist);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  Pecos::ProbabilityTransformation& probability_transformation();

  //bool initialize_mapping(ParLevLIter pl_iter);
  //bool finalize_mapping();
  bool resize_pending() const;
  void update_from_subordinate_model(size_t depth = SZ_MAX);

  /// set primaryACVarMapIndices and secondaryACVarMapTargets (only, for now)
  void nested_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2);
  /// return primaryACVarMapIndices
  const SizetArray& nested_acv1_indices() const;
  /// return secondaryACVarMapTargets
  const ShortArray& nested_acv2_targets() const;
  
  /// calculate and return potential state of distribution parameter
  /// derivatives, but do not cache value in distParamDerivs
  short query_distribution_parameter_derivatives() const;
  /// activate distParamDerivs to {NO,MIXED,ALL}_DERIVS
  void activate_distribution_parameter_derivatives();
  /// reset distParamDerivs to NO_DERIVS
  void deactivate_distribution_parameter_derivatives();

  void assign_instance();

  void init_metadata() override { /* no-op to leave metadata intact */}

  void trans_U_to_X(const RealVector& u_c_vars, RealVector& x_c_vars);
  void trans_U_to_X(const Variables&  u_vars,   Variables&  x_vars);
  void trans_X_to_U(const RealVector& x_c_vars, RealVector& u_c_vars);
  void trans_X_to_U(const Variables&  x_vars,   Variables&  u_vars);

  void trans_grad_X_to_U(const RealVector& fn_grad_x, RealVector& fn_grad_u,
			 const RealVector& x_vars);
  void trans_grad_U_to_X(const RealVector& fn_grad_u, RealVector& fn_grad_x,
			 const RealVector& x_vars);
  void trans_grad_X_to_S(const RealVector& fn_grad_x, RealVector& fn_grad_s,
			 const RealVector& x_vars);
  void trans_hess_X_to_U(const RealSymMatrix& fn_hess_x,
			 RealSymMatrix& fn_hess_u, const RealVector& x_vars,
			 const RealVector& fn_grad_x);

  //
  //- Heading: Member functions
  //

  /// initialize transformed distribution types and natafTransform
  /// (construct time)
  void initialize_transformation(short u_space_type);
  /// update with latest distribution data (run time)
  void update_transformation();

  /// instantiate and initialize natafTransform
  void initialize_nataf();

  // x-space correlations assigned in Model and u-space is uncorrelated
  //void update_distribution_correlations();
  /// verify that correlation warping is supported by Nataf for given
  /// variable types
  void verify_correlation_support(short u_space_type);

  /// initialize the continuous/discrete variable types using u-space types
  /// (converted from Pecos to Dakota)
  void initialize_dakota_variable_types();
  /// update model bounds using u-space (truncated) distribution bounds
  void update_model_bounds(bool truncate_bnds, Real bnd);

  /// detect when the variables transformation is nonlinear
  bool nonlinear_variables_mapping(
    const Pecos::MultivariateDistribution& x_dist,
    const Pecos::MultivariateDistribution& u_dist) const;

  /// convert vector<RandomVariable> index to active correlation index
  size_t rv_index_to_corr_index(size_t rv_index);
  /// convert allContinuousVars index to active correlation index
  size_t acv_index_to_corr_index(size_t acv_index);

  /// convert from Pecos To Dakota variable enumeration type for continuous
  /// aleatory uncertain variables used in variable transformations
  unsigned short pecos_to_dakota_variable_type(unsigned short pecos_var_type,
					       size_t rv_index);

  /// static function for RecastModels used for forward mapping of u-space
  /// variables from NonD Iterators to x-space variables for Model evaluations
  static void vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars);
  /// static function for RecastModels used for inverse mapping of x-space
  /// variables from data import to u-space variables for NonD Iterators
  static void vars_x_to_u_mapping(const Variables& x_vars, Variables& u_vars);

  /// static function for RecastModels used to map u-space ActiveSets
  /// from NonD Iterators to x-space ActiveSets for Model evaluations
  static void set_u_to_x_mapping(const Variables& u_vars,
                                 const ActiveSet& u_set, ActiveSet& x_set);

  /// static function for RecastModels used to map x-space responses from
  /// Model evaluations to u-space responses for return to NonD Iterator.
  static void resp_x_to_u_mapping(const Variables& x_vars,
                                  const Variables& u_vars,
                                  const Response& x_response,
                                  Response& u_response);

private:

  /// Nonlinear variable transformation that encapsulates the required
  /// data for performing transformations from X -> Z -> U and back.
  Pecos::ProbabilityTransformation natafTransform;

  /// indicates state of derivatives of final results with respect to
  /// distribution parameters s within resp_x_to_u_mapping() using the chain
  /// rule df/dx dx/ds.  The default is to calculate derivatives with respect
  /// to standard random variables u using the chain rule df/dx dx/du.
  short distParamDerivs;

  /// boolean flag indicating use of distribution truncation for
  /// defining global model bounds
  bool truncatedBounds;
  /// number of +/- standard deviations used for defining bounds truncation
  Real boundVal;

  /// "primary" all continuous variable mapping indices flowed down
  /// from higher level iteration
  SizetArray primaryACVarMapIndices;
  // "primary" all discrete int variable mapping indices flowed down from
  // higher level iteration
  //SizetArray primaryADIVarMapIndices;
  // "primary" all discrete string variable mapping indices flowed down from
  // higher level iteration
  //SizetArray primaryADSVarMapIndices;
  // "primary" all discrete real variable mapping indices flowed down from
  // higher level iteration
  //SizetArray primaryADRVarMapIndices;
  /// "secondary" all continuous variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryACVarMapTargets;
  // "secondary" all discrete int variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryADIVarMapTargets;
  // "secondary" all discrete string variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryADSVarMapTargets;
  // "secondary" all discrete real variable mapping targets flowed down
  // from higher level iteration
  //ShortArray secondaryADRVarMapTargets;

  /// static pointer to an active instance of this class for use in
  /// static function callbacks
  static ProbabilityTransformModel* ptmInstance;
};


inline void ProbabilityTransformModel::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  primaryACVarMapIndices      = c_index1;
  //primaryADIVarMapIndices   = di_index1;
  //primaryADSVarMapIndices   = ds_index1;
  //primaryADRVarMapIndices   = dr_index1;
  secondaryACVarMapTargets    = c_target2;
  //secondaryADIVarMapTargets = di_target2;
  //secondaryADSVarMapTargets = ds_target2;
  //secondaryADRVarMapTargets = dr_target2;
}


inline short ProbabilityTransformModel::
query_distribution_parameter_derivatives() const
{
  short dist_param_derivs = NO_DERIVS;
  size_t i, num_outer_cv = secondaryACVarMapTargets.size();
  if (num_outer_cv) {
    bool tgt = false, no_tgt = false;
    for (i=0; i<num_outer_cv; ++i)
      if (secondaryACVarMapTargets[i] == Pecos::NO_TARGET) no_tgt = true;
      else                                                    tgt = true;
    if (tgt && no_tgt) dist_param_derivs = MIXED_DERIVS;
    else if (tgt)      dist_param_derivs =   ALL_DERIVS;
  }
  return dist_param_derivs;
}


inline void ProbabilityTransformModel::
activate_distribution_parameter_derivatives()
{ distParamDerivs = query_distribution_parameter_derivatives(); }


inline void ProbabilityTransformModel::
deactivate_distribution_parameter_derivatives()
{ distParamDerivs = NO_DERIVS; }


inline const SizetArray& ProbabilityTransformModel::nested_acv1_indices() const
{ return primaryACVarMapIndices; }


inline const ShortArray& ProbabilityTransformModel::nested_acv2_targets() const
{ return secondaryACVarMapTargets; }


inline bool ProbabilityTransformModel::resize_pending() const
{ return subModel.resize_pending(); }


inline void ProbabilityTransformModel::initialize_nataf()
{
  if (natafTransform.is_null()) {
    natafTransform = Pecos::ProbabilityTransformation("nataf"); // for now
    // shallow copies
    natafTransform.x_distribution(subModel.multivariate_distribution());
    natafTransform.u_distribution(mvDist);
  }
}


inline void ProbabilityTransformModel::update_transformation()
{
  mvDist.pull_distribution_parameters(subModel.multivariate_distribution());
  // x-space correlations assigned in Model and u-space is uncorrelated
  //update_distribution_correlations();

  // Modify the correlation matrix (Nataf) and compute its Cholesky factor.
  // Since the uncertain variable distributions (means, std devs, correlations)
  // may change, update of correlation warpings is performed regularly.
  natafTransform.transform_correlations();

  update_model_bounds(truncatedBounds, boundVal);
}


inline void ProbabilityTransformModel::
initialize_transformation(short u_space_type)
{
  if (mvDist.is_null()) // already initialized: no current reason to update
    mvDist = Pecos::MultivariateDistribution(Pecos::MARGINALS_CORRELATIONS);

  // Follows init_sizes() since pulls view from currentVariables.
  // Precedes initialize_distribution_types() since inactive not transformed.
  initialize_active_types(mvDist);
  // now initialized based on Model view, can use u-space active subset
  // (which may differ from x-space) for transformation
  initialize_distribution_types(u_space_type, mvDist.active_variables(),
				subModel.multivariate_distribution(), mvDist);
  initialize_nataf();
  initialize_dakota_variable_types();
  verify_correlation_support(u_space_type);

  // also perform run-time update as some construct-time operations require it
  // (e.g. grid_size() for maxConcurrency, inverse vars transform in ctor).
  update_transformation();
}


inline void ProbabilityTransformModel::
update_from_subordinate_model(size_t depth)
{
  // standard updates for RecastModels, including subModel recursion
  //RecastModel::update_from_subordinate_model(depth);
  // ordering problem with invMapping dependence on dist params

  // data flows from the bottom-up, so recurse first
  if (depth == SZ_MAX)
    subModel.update_from_subordinate_model(depth); // retain special value (inf)
  else if (depth)
    subModel.update_from_subordinate_model(depth - 1); // decrement
  //else depth exhausted --> update this level only

  // propagate any subModel parameter updates to mvDist
  update_transformation();

  // now pull additional updates from subModel (requires latest dist params)
  //RecastModel::update_from_model(subModel);
  // here we override the logic in RecastModel::update_response_from_model()
  // (which suppresses updates if {primary,secondary}RespMapping) since we want
  // to include primary/secondary response updates for ProbabilityTransforms
  // (the response transform is 1-to-1 and only involves derivative mappings)
  bool update_active_complement = update_variables_from_model(subModel);
  if (update_active_complement)
    update_variables_active_complement_from_model(subModel);
  update_primary_response(subModel);
  update_secondary_response(subModel);
}


inline bool ProbabilityTransformModel::
nonlinear_variables_mapping(const Pecos::MultivariateDistribution& x_dist,
			    const Pecos::MultivariateDistribution& u_dist) const
{
  bool nln_vars_map = false;
  const ShortArray& x_types = x_dist.random_variable_types();
  const ShortArray& u_types = u_dist.random_variable_types();
  size_t i, num_types = std::min(x_types.size(), u_types.size());
  const BitArray& active_v = u_dist.active_variables();
  for (i=0; i<num_types; ++i)
    if (active_v[i]) {
      switch (u_types[i]) {
      case Pecos::STD_NORMAL:
	nln_vars_map = (x_types[i] != Pecos::NORMAL); break;
      case Pecos::STD_UNIFORM:
	switch (x_types[i]) {
	case Pecos::CONTINUOUS_RANGE: case Pecos::UNIFORM:
	case Pecos::HISTOGRAM_BIN:    case Pecos::CONTINUOUS_INTERVAL_UNCERTAIN:
	  break;
	default:  nln_vars_map = true;  break;
	}
	break;
      case Pecos::STD_EXPONENTIAL:
	nln_vars_map = (x_types[i] != Pecos::EXPONENTIAL); break;
      case Pecos::STD_BETA:  nln_vars_map = (x_types[i] != Pecos::BETA);  break;
      case Pecos::STD_GAMMA: nln_vars_map = (x_types[i] != Pecos::GAMMA); break;
      default:               nln_vars_map = (x_types[i] != u_types[i]);   break;
      }

      if (nln_vars_map) break;
    }

  return nln_vars_map;
}


inline Pecos::ProbabilityTransformation& ProbabilityTransformModel::
probability_transformation()
{ return natafTransform; }


inline void ProbabilityTransformModel::assign_instance()
{ ptmInstance = this; }


/** Map the variables from iterator space (u) to simulation space (x). */
inline void ProbabilityTransformModel::
trans_U_to_X(const RealVector& u_c_vars, RealVector& x_c_vars)
{
  const Variables& x_vars = subModel.current_variables();
  short u_active_view = currentVariables.shared_data().view().first,
        x_active_view = x_vars.shared_data().view().first;

  // Note: the cv ids for x and u should be identical following view alignment,
  // but we pass both for generality
  if (u_active_view == x_active_view)
    natafTransform.trans_U_to_X(u_c_vars,
      currentVariables.continuous_variable_ids(), x_c_vars,
      x_vars.continuous_variable_ids());
  else {
    bool u_all = (u_active_view == RELAXED_ALL || u_active_view == MIXED_ALL),
         x_all = (x_active_view == RELAXED_ALL || x_active_view == MIXED_ALL);
    if (!u_all && x_all)
      natafTransform.trans_U_to_X(u_c_vars,
	currentVariables.all_continuous_variable_ids(), x_c_vars,
	x_vars.continuous_variable_ids());
    else if (!x_all && u_all)
      natafTransform.trans_U_to_X(u_c_vars,
	currentVariables.continuous_variable_ids(), x_c_vars,
	x_vars.all_continuous_variable_ids());
    else {
      Cerr << "Error: unsupported variable view differences in "
	   << "ProbabilityTransformModel::trans_U_to_X()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  // *** TO DO: active discrete {int,string,real}
}


/** Map the variables from iterator space (u) to simulation space (x). */
inline void ProbabilityTransformModel::
trans_U_to_X(const Variables& u_vars, Variables& x_vars)
{
  short u_active_view = u_vars.shared_data().view().first,
        x_active_view = x_vars.shared_data().view().first;

  // Note: the cv ids for x and u should be identical following view alignment,
  // but we pass both for generality
  if (u_active_view == x_active_view)
    natafTransform.trans_U_to_X(
      u_vars.continuous_variables(),      u_vars.continuous_variable_ids(),
      x_vars.continuous_variables_view(), x_vars.continuous_variable_ids());
  else {
    bool u_all = (u_active_view == RELAXED_ALL || u_active_view == MIXED_ALL),
         x_all = (x_active_view == RELAXED_ALL || x_active_view == MIXED_ALL);
    if (!u_all && x_all)
      natafTransform.trans_U_to_X(
        u_vars.all_continuous_variables(), u_vars.all_continuous_variable_ids(),
	x_vars.continuous_variables_view(), x_vars.continuous_variable_ids());
    else if (!x_all && u_all) {
      RealVector x_acv;
      natafTransform.trans_U_to_X(
	u_vars.continuous_variables(), u_vars.continuous_variable_ids(),
	x_acv, x_vars.all_continuous_variable_ids());
      x_vars.all_continuous_variables(x_acv);
    }
    else {
      Cerr << "Error: unsupported variable view differences in "
	   << "ProbabilityTransformModel::trans_U_to_X()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  // *** TO DO: active discrete {int,string,real}
}


/** Map the variables from simulation space (x) to iterator space (u). */
inline void ProbabilityTransformModel::
trans_X_to_U(const RealVector& x_c_vars, RealVector& u_c_vars)
{
  const Variables& x_vars = subModel.current_variables();
  short u_active_view = currentVariables.shared_data().view().first,
        x_active_view = x_vars.shared_data().view().first;

  if (u_active_view == x_active_view)
    natafTransform.trans_X_to_U(x_c_vars, x_vars.continuous_variable_ids(),
      u_c_vars, currentVariables.continuous_variable_ids());
  else {
    bool u_all = (u_active_view == RELAXED_ALL || u_active_view == MIXED_ALL),
         x_all = (x_active_view == RELAXED_ALL || x_active_view == MIXED_ALL);
    if (!u_all && x_all)
      natafTransform.trans_X_to_U(x_c_vars, x_vars.continuous_variable_ids(),
	u_c_vars, currentVariables.all_continuous_variable_ids());
    else if (!x_all && u_all)
      natafTransform.trans_X_to_U(x_c_vars,x_vars.all_continuous_variable_ids(),
	u_c_vars, currentVariables.continuous_variable_ids());
    else {
      Cerr << "Error: unsupported variable view differences in "
	   << "ProbabilityTransformModel::trans_X_to_U()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  // *** TO DO: active discrete {int,string,real}
}


/** Map the variables from simulation space (x) to iterator space (u). */
inline void ProbabilityTransformModel::
trans_X_to_U(const Variables& x_vars, Variables& u_vars)
{
  short u_active_view = u_vars.shared_data().view().first,
        x_active_view = x_vars.shared_data().view().first;

  if (u_active_view == x_active_view)
    natafTransform.trans_X_to_U(
      x_vars.continuous_variables(),      x_vars.continuous_variable_ids(),
      u_vars.continuous_variables_view(), u_vars.continuous_variable_ids());
  else {
    bool u_all = (u_active_view == RELAXED_ALL || u_active_view == MIXED_ALL),
         x_all = (x_active_view == RELAXED_ALL || x_active_view == MIXED_ALL);
    if (!u_all && x_all) {
      RealVector u_acv;
      natafTransform.trans_X_to_U(
	x_vars.continuous_variables(), x_vars.continuous_variable_ids(),
	u_acv,                         u_vars.all_continuous_variable_ids());
      u_vars.all_continuous_variables(u_acv);
    }
    else if (!x_all && u_all)
      natafTransform.trans_X_to_U(
	x_vars.all_continuous_variables(), x_vars.all_continuous_variable_ids(),
	u_vars.continuous_variables_view(), u_vars.continuous_variable_ids());
    else {
      Cerr << "Error: unsupported variable view differences in "
	   << "ProbabilityTransformModel::trans_X_to_U()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
  // *** TO DO: active discrete {int,string,real}
}


/** Map the variables from iterator space (u) to simulation space (x). */
inline void ProbabilityTransformModel::
vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars)
{ ptmInstance->trans_U_to_X(u_vars, x_vars); }


/** Map the variables from simulation space (x) to iterator space (u). */
inline void ProbabilityTransformModel::
vars_x_to_u_mapping(const Variables& x_vars, Variables& u_vars)
{ ptmInstance->trans_X_to_U(x_vars, u_vars); }


inline void ProbabilityTransformModel::
trans_grad_X_to_U(const RealVector& fn_grad_x, RealVector& fn_grad_u,
		  const RealVector& x_vars)
{
  SizetMultiArrayConstView x_cv_ids = subModel.continuous_variable_ids(),
    u_cv_ids = currentVariables.continuous_variable_ids();
  SizetArray x_dvv; copy_data(x_cv_ids, x_dvv);
  natafTransform.trans_grad_X_to_U(fn_grad_x, x_cv_ids, fn_grad_u, u_cv_ids,
				   x_vars, x_dvv);
}


inline void ProbabilityTransformModel::
trans_grad_U_to_X(const RealVector& fn_grad_u, RealVector& fn_grad_x,
		  const RealVector& x_vars)
{
  SizetMultiArrayConstView x_cv_ids = subModel.continuous_variable_ids(),
    u_cv_ids = currentVariables.continuous_variable_ids();
  SizetArray x_dvv; copy_data(x_cv_ids, x_dvv);
  natafTransform.trans_grad_U_to_X(fn_grad_u, u_cv_ids, fn_grad_x, x_cv_ids,
				   x_vars, x_dvv);
}


inline void ProbabilityTransformModel::
trans_grad_X_to_S(const RealVector& fn_grad_x, RealVector& fn_grad_s,
		  const RealVector& x_vars)
{
  SizetMultiArrayConstView x_cv_ids = subModel.continuous_variable_ids();
  SizetArray x_dvv; copy_data(x_cv_ids, x_dvv);
  natafTransform.trans_grad_X_to_S(fn_grad_x, fn_grad_s, x_vars, x_dvv,
    x_cv_ids, currentVariables.continuous_variable_ids(), // u_cv_ids
    subModel.all_continuous_variable_ids(),              // x_acv_ids
    primaryACVarMapIndices, secondaryACVarMapTargets);
}


inline void ProbabilityTransformModel::
trans_hess_X_to_U(const RealSymMatrix& fn_hess_x, RealSymMatrix& fn_hess_u,
		  const RealVector& x_vars, const RealVector& fn_grad_x)
{
  SizetMultiArrayConstView x_cv_ids = subModel.continuous_variable_ids(),
    u_cv_ids = currentVariables.continuous_variable_ids();
  SizetArray x_dvv; copy_data(x_cv_ids, x_dvv);
  natafTransform.trans_hess_X_to_U(fn_hess_x, x_cv_ids, fn_hess_u, u_cv_ids,
				   x_vars, fn_grad_x, x_dvv);
}


inline size_t ProbabilityTransformModel::
rv_index_to_corr_index(size_t rv_index)
{
  const Pecos::BitArray& active_corr
    = subModel.multivariate_distribution().active_correlations();
  if (active_corr.empty())
    return rv_index; // no mask
  else if (active_corr[rv_index]) { // offset RV index to account for mask
    size_t i, corr_index = 0;
    for (i=0; i<rv_index; ++i)
      if (active_corr[i])
	++corr_index;
    return corr_index;
  }
  else // RV not active in correlated variable subset
    return _NPOS;
}


inline size_t ProbabilityTransformModel::
acv_index_to_corr_index(size_t acv_index)
{
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  return rv_index_to_corr_index(svd.acv_index_to_all_index(acv_index));
}

} // namespace Dakota

#endif
