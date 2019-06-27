/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ProbabilityTransformModel
//- Description: Specialization of RecastModel to transform a sub-model to
//-              u-space.
//- Owner:       Brian Adams
//- Checked by:
//- Version: $Id$

#ifndef PROBABILITY_TRANSFORM_MODEL_H
#define PROBABILITY_TRANSFORM_MODEL_H

#include "RecastModel.hpp"
#include "ProbabilityTransformation.hpp"
#include "pecos_stat_util.hpp"

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
                            bool truncate_bnds = false, Real bnd = 10.);

  /// destructor
  ~ProbabilityTransformModel();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  Pecos::ProbabilityTransformation& probability_transformation();

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();
  bool resize_pending() const;
  void update_from_subordinate_model(size_t depth =
				     std::numeric_limits<size_t>::max());

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
  // alternate form: initialize natafTransform based on incoming data
  //void initialize_nataf(const Pecos::ProbabilityTransformation& transform,
  //			bool deep_copy = false);

  /// initialize transformed distribution types and instantiate mvDist
  void initialize_distribution_types(short u_space_type);

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

  /// set distParamDerivs
  void distribution_parameter_derivatives(bool dist_param_derivs);

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

  /// flags calculation of derivatives with respect to distribution
  /// parameters s within resp_x_to_u_mapping() using the chain rule
  /// df/dx dx/ds.  The default is to calculate derivatives with respect
  /// to standard random variables u using the chain rule df/dx dx/du.
  bool distParamDerivs;

  /// boolean flag indicating use of distribution truncation for
  /// defining global model bounds
  bool truncatedBounds;
  /// number of +/- standard deviations used for defining bounds truncation
  Real boundVal;

  /// "primary" all continuous variable mapping indices flowed down
  /// from higher level iteration
  SizetArray primaryACVarMapIndices;
  /// "secondary" all continuous variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryACVarMapTargets;

  /// static pointer to this class for use in static callbacks
  static ProbabilityTransformModel* ptmInstance;
};


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


/* This function is commonly used to publish tranformation data when
   the Model variables are in a transformed space (e.g., u-space) and
   ProbabilityTransformation::ranVarTypes et al. may not be generated
   directly.  This allows for the use of inverse transformations to
   return the transformed space variables to their original states.
void ProbabilityTransformModel::
initialize_nataf(const Pecos::ProbabilityTransformation& transform,
		 bool deep_copy)
{
  if (deep_copy) {
    initialize_nataf();
    natafTransform.copy(transform);
    // TO DO: deep copy of randomVarsX not yet implemented in
    // Pecos::ProbabilityTransformation::copy()
  }
  else
    natafTransform = transform; // shared rep
}
*/


inline void ProbabilityTransformModel::
initialize_transformation(short u_space_type)
{
  if (mvDist.is_null()) // already initialized: no current reason to update
    mvDist = Pecos::MultivariateDistribution(Pecos::MARGINALS_CORRELATIONS);

  initialize_distribution_types(u_space_type);
  initialize_nataf();
  initialize_dakota_variable_types();
  verify_correlation_support(u_space_type);
  // defer distribution parameter updates until run time (see below)
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
update_from_subordinate_model(size_t depth)
{
  // standard updates for RecastModels, including subModel recursion
  //RecastModel::update_from_subordinate_model(depth);
  // ordering problem with invMapping dependence on dist params

  // data flows from the bottom-up, so recurse first
  if (depth == std::numeric_limits<size_t>::max())
    subModel.update_from_subordinate_model(depth); // retain special value (inf)
  else if (depth)
    subModel.update_from_subordinate_model(depth - 1); // decrement
  //else depth exhausted --> update this level only

  // propagate any subModel parameter updates to mvDist
  update_transformation();

  // now pull additional updates from subModel (requires latest dist params)
  RecastModel::update_from_model(subModel);
}


inline bool ProbabilityTransformModel::
nonlinear_variables_mapping(const Pecos::MultivariateDistribution& x_dist,
			    const Pecos::MultivariateDistribution& u_dist) const
{
  bool nln_vars_map = false;
  const ShortArray& x_types = x_dist.random_variable_types();
  const ShortArray& u_types = u_dist.random_variable_types();
  size_t i, num_types = std::min(x_types.size(), u_types.size());
  const BitArray& active_v = x_dist.active_variables();
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


inline void ProbabilityTransformModel::
distribution_parameter_derivatives(bool dist_param_derivs)
{ distParamDerivs = dist_param_derivs; }


/** Map the variables from iterator space (u) to simulation space (x). */
inline void ProbabilityTransformModel::
vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars)
{
  ptmInstance->natafTransform.trans_U_to_X(u_vars.continuous_variables(),
					   x_vars.continuous_variables_view());
  // *** TO DO: active discrete {int,string,real}
}


/** Map the variables from simulation space (x) to iterator space (u). */
inline void ProbabilityTransformModel::
vars_x_to_u_mapping(const Variables& x_vars, Variables& u_vars)
{
  ptmInstance->natafTransform.trans_X_to_U(x_vars.continuous_variables(),
					   u_vars.continuous_variables_view());
  // *** TO DO: active discrete {int,string,real}
}


inline Pecos::ProbabilityTransformation& ProbabilityTransformModel::
probability_transformation()
{ return natafTransform; }

} // namespace Dakota

#endif
