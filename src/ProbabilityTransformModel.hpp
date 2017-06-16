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
  ProbabilityTransformModel(const Model& sub_model,
                            bool truncated_bounds = false, Real bound = 10.);

  /// destructor
  ~ProbabilityTransformModel();

  //
  //- Heading: Virtual function redefinitions
  //

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();
  bool mapping_initialized() const;
  bool resize_pending() const;

  //
  //- Heading: Member functions
  //

  /// perform correlation warping for variable types supported by Nataf
  void transform_correlations();

protected:

  //
  //- Heading: Member functions
  //

  /// set distParamDerivs
  void distribution_parameter_derivatives(bool dist_param_derivs);

  /// alternate form: initialize natafTransform based on incoming data
  void initialize_random_variables(
    const Pecos::ProbabilityTransformation& transform, bool deep_copy = false);

  /// instantiate natafTransform
  void initialize_random_variable_transformation();

  /// initializes ranVarTypesX within natafTransform (u-space not needed)
  void initialize_random_variable_types();

  /// initializes ranVarTypesX and ranVarTypesU within natafTransform
  void initialize_random_variable_types(short u_space_type);

  /// initializes ranVarMeansX, ranVarStdDevsX, ranVarLowerBndsX,
  /// ranVarUpperBndsX, and ranVarAddtlParamsX within natafTransform
  void initialize_random_variable_parameters();

  /// propagate iteratedModel correlations to natafTransform
  void initialize_random_variable_correlations();

  /// verify that correlation warping is supported by Nataf for given
  /// variable types
  void verify_correlation_support(short u_space_type);

  /// recast x_model from x-space to u-space to create u_model
  void transform_model(bool truncated_bounds, Real bound);

  /// convert from Pecos To Dakota variable enumeration type for continuous
  /// aleatory uncertain variables used in variable transformations
  unsigned short pecos_to_dakota_variable_type(unsigned short pecos_var_type);


  void initialize_sizes();

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

  // The following variable counts reflect the native Model space, which could
  // correspond to either X or U space.  If a specific X or U variables count
  // is needed, then natafTransform.ranVarTypesX/U.count() should be used.

  size_t numFunctions;          ///< number of response functions
  size_t numContinuousVars;     ///< number of active continuous vars
  size_t numDiscreteIntVars;    ///< number of active discrete integer vars
  size_t numDiscreteStringVars; ///< number of active discrete string vars
  size_t numDiscreteRealVars;   ///< number of active discrete real vars

  /// number of continuous design variables (modeled using uniform
  /// distribution for All view modes)
  size_t numContDesVars;
  /// number of discrete integer design variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscIntDesVars;
  /// number of discrete string design variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscStringDesVars;
  /// number of discrete real design variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscRealDesVars;
  /// total number of design variables
  size_t numDesignVars;
  /// number of continuous state variables (modeled using uniform
  /// distribution for All view modes)
  size_t numContStateVars;
  /// number of discrete integer state variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscIntStateVars;
  /// number of discrete string state variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscStringStateVars;
  /// number of discrete real state variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscRealStateVars;
  /// total number of state variables
  size_t numStateVars;

  /// number of normal uncertain variables (native space)
  size_t numNormalVars;
  /// number of lognormal uncertain variables (native space)
  size_t numLognormalVars;
  /// number of uniform uncertain variables (native space)
  size_t numUniformVars;
  /// number of loguniform uncertain variables (native space)
  size_t numLoguniformVars;
  /// number of triangular uncertain variables (native space)
  size_t numTriangularVars;
  /// number of exponential uncertain variables (native space)
  size_t numExponentialVars;
  /// number of beta uncertain variables (native space)
  size_t numBetaVars;
  /// number of gamma uncertain variables (native space)
  size_t numGammaVars;
  /// number of gumbel uncertain variables (native space)
  size_t numGumbelVars;
  /// number of frechet uncertain variables (native space)
  size_t numFrechetVars;
  /// number of weibull uncertain variables (native space)
  size_t numWeibullVars;
  /// number of histogram bin uncertain variables (native space)
  size_t numHistogramBinVars;
  /// number of Poisson uncertain variables (native space)
  size_t numPoissonVars;
  /// number of binomial uncertain variables (native space)
  size_t numBinomialVars;
  /// number of negative binomial uncertain variables (native space)
  size_t numNegBinomialVars;
  /// number of geometric uncertain variables (native space)
  size_t numGeometricVars;
  /// number of hypergeometric uncertain variables (native space)
  size_t numHyperGeomVars;
  /// number of histogram point integer uncertain variables (native space)
  size_t numHistogramPtIntVars;
  /// number of histogram point string uncertain variables (native space)
  size_t numHistogramPtStringVars;
  /// number of histogram point real uncertain variables (native space)
  size_t numHistogramPtRealVars;
  /// number of continuous interval uncertain variables (native space)
  size_t numContIntervalVars;
  /// number of discrete interval uncertain variables (native space)
  size_t numDiscIntervalVars;
  /// number of discrete integer set uncertain variables (native space)
  size_t numDiscSetIntUncVars;
  /// number of discrete integer set uncertain variables (native space)
  size_t numDiscSetStringUncVars;
  /// number of discrete real set uncertain variables (native space)
  size_t numDiscSetRealUncVars;

  /// total number of continuous aleatory uncertain variables (native space)
  size_t numContAleatUncVars;
  /// total number of discrete integer aleatory uncertain variables
  /// (native space)
  size_t numDiscIntAleatUncVars;
  /// total number of discrete string aleatory uncertain variables
  /// (native space)
  size_t numDiscStringAleatUncVars;
  /// total number of discrete real aleatory uncertain variables (native space)
  size_t numDiscRealAleatUncVars;
  /// total number of aleatory uncertain variables (native space)
  size_t numAleatoryUncVars;
  /// total number of continuous epistemic uncertain variables (native space)
  size_t numContEpistUncVars;
  /// total number of discrete integer epistemic uncertain variables
  /// (native space)
  size_t numDiscIntEpistUncVars;
  /// total number of discrete string epistemic uncertain variables
  /// (native space)
  size_t numDiscStringEpistUncVars;
  /// total number of discrete real epistemic uncertain variables (native space)
  size_t numDiscRealEpistUncVars;
  /// total number of epistemic uncertain variables (native space)
  size_t numEpistemicUncVars;
  /// total number of uncertain variables (native space)
  size_t numUncertainVars;

  /// flags calculation of derivatives with respect to distribution
  /// parameters s within resp_x_to_u_mapping() using the chain rule
  /// df/dx dx/ds.  The default is to calculate derivatives with respect
  /// to standard random variables u using the chain rule df/dx dx/du.
  bool distParamDerivs;

  /// boolean flag to indicate truncated bounds
  bool truncatedBounds;

  /// bound value
  Real boundVal;

  /// flag for computing interval-type metrics instead of integrated
  /// metrics If any epistemic variables are active in a metric
  /// evaluation, then this flag is set.
  bool epistemicStats;

  /// "primary" all continuous variable mapping indices flowed down
  /// from higher level iteration
  SizetArray primaryACVarMapIndices;
  /// "secondary" all continuous variable mapping targets flowed down
  /// from higher level iteration
  ShortArray secondaryACVarMapTargets;

  /// static pointer to this class for use in static callbacks
  static ProbabilityTransformModel* ptmInstance;

  /// track use of initialize_mapping() and finalize_mapping()
  bool mappingInitialized;
};


inline bool ProbabilityTransformModel::mapping_initialized() const
{ return mappingInitialized; }


inline bool ProbabilityTransformModel::resize_pending() const
{ return subModel.resize_pending(); }


inline void ProbabilityTransformModel::transform_correlations()
{ natafTransform.transform_correlations(); }


inline void ProbabilityTransformModel::
distribution_parameter_derivatives(bool dist_param_derivs)
{ distParamDerivs = dist_param_derivs; }


/** Map the variables from iterator space (u) to simulation space (x). */
inline void ProbabilityTransformModel::
vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars)
{
  ptmInstance->natafTransform.trans_U_to_X(u_vars.continuous_variables(),
      x_vars.continuous_variables_view());
}


/** Map the variables from simulation space (x) to iterator space (u). */
inline void ProbabilityTransformModel::
vars_x_to_u_mapping(const Variables& x_vars, Variables& u_vars)
{
  ptmInstance->natafTransform.trans_X_to_U(x_vars.continuous_variables(),
      u_vars.continuous_variables_view());
}

} // namespace Dakota

#endif
