/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataVariables
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataVariables.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DATA_VARIABLES_H
#define DATA_VARIABLES_H

#include "system_defs.h"
#include "global_defs.h"  // for Cerr
#include "data_types.h"
#include "DakotaString.H"

namespace Dakota {

// special values for domain: Relaxed or Mixed
enum { DEFAULT_DOMAIN, RELAXED_DOMAIN, MIXED_DOMAIN };
// special values for active view: All, Design, {A+E,A,E} Uncertain, or State
enum { DEFAULT_VIEW, ALL_VIEW, DESIGN_VIEW, UNCERTAIN_VIEW,
       ALEATORY_UNCERTAIN_VIEW, EPISTEMIC_UNCERTAIN_VIEW, STATE_VIEW };
// special values for active and inactive views include EMPTY plus the 12
// combinations of All, Design, {A+E,A,E} Uncertain, or State active view
// [6 total] and Relaxed or Mixed domain [2 total].
enum { EMPTY, RELAXED_ALL, MIXED_ALL,
       RELAXED_DESIGN, RELAXED_UNCERTAIN, RELAXED_ALEATORY_UNCERTAIN,
       RELAXED_EPISTEMIC_UNCERTAIN, RELAXED_STATE,
       MIXED_DESIGN, MIXED_UNCERTAIN, MIXED_ALEATORY_UNCERTAIN,
       MIXED_EPISTEMIC_UNCERTAIN, MIXED_STATE };

// special values for SharedVariablesData::variablesComponents and
// SharedVariablesData::all*Types (Note: these differ from the
// pecos_global_defs.hpp enumeration, which is focused on ranVarTypesX/U)
enum { CONTINUOUS_DESIGN, DISCRETE_DESIGN_RANGE, DISCRETE_DESIGN_SET_INT,
       DISCRETE_DESIGN_SET_REAL, NORMAL_UNCERTAIN, LOGNORMAL_UNCERTAIN,
       UNIFORM_UNCERTAIN, LOGUNIFORM_UNCERTAIN, TRIANGULAR_UNCERTAIN,
       EXPONENTIAL_UNCERTAIN, BETA_UNCERTAIN, GAMMA_UNCERTAIN, GUMBEL_UNCERTAIN,
       FRECHET_UNCERTAIN, WEIBULL_UNCERTAIN, HISTOGRAM_BIN_UNCERTAIN,
       POISSON_UNCERTAIN, BINOMIAL_UNCERTAIN, NEGATIVE_BINOMIAL_UNCERTAIN,
       GEOMETRIC_UNCERTAIN, HYPERGEOMETRIC_UNCERTAIN, HISTOGRAM_POINT_UNCERTAIN,
       CONTINUOUS_INTERVAL_UNCERTAIN, DISCRETE_INTERVAL_UNCERTAIN,
       DISCRETE_UNCERTAIN_SET_INT, DISCRETE_UNCERTAIN_SET_REAL,
       CONTINUOUS_STATE, DISCRETE_STATE_RANGE, DISCRETE_STATE_SET_INT,
       DISCRETE_STATE_SET_REAL };


/// Body class for variables specification data.

/** The DataVariablesRep class is used to contain the data from a
    variables keyword specification.  Default values are managed in
    the DataVariablesRep constructor.  Data is public to avoid
    maintaining set/get functions, but is still encapsulated within
    ProblemDescDB since ProblemDescDB::dataVariablesList is private. */

class DataVariablesRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataVariables;

//private:
public:

  //
  //- Heading: Data
  //

  /// string identifier for the variables specification data set
  /// (from the \c id_variables specification in \ref VarSetId)
  String idVariables;
  /// user selection/override of variables view: {DEFAULT,ALL,DESIGN,
  /// UNCERTAIN,ALEATORY_UNCERTAIN,EPISTEMIC_UNCERTAIN,STATE}_VIEW
  short varsView;
  /// user selection/override of variables domain:
  /// {DEFAULT,MIXED,RELAXED}_DOMAIN
  short varsDomain;
  /// number of continuous design variables (from the \c continuous_design
  /// specification in \ref VarDV)
  size_t numContinuousDesVars;
  /// number of discrete design variables defined by an integer range
  /// (from the \c discrete_design_range specification in \ref VarDV)
  size_t numDiscreteDesRangeVars;
  /// number of discrete design variables defined by a set of integers
  /// (from the \c discrete_design_set_integer specification in \ref VarDV)
  size_t numDiscreteDesSetIntVars;
  /// number of discrete design variables defined by a set of reals
  /// (from the \c discrete_design_set_real specification in \ref VarDV)
  size_t numDiscreteDesSetRealVars;
  /// number of normal uncertain variables (from the \c normal_uncertain
  /// specification in \ref VarAUV)
  size_t numNormalUncVars;
  /// number of lognormal uncertain variables (from the \c lognormal_uncertain
  /// specification in \ref VarAUV)
  size_t numLognormalUncVars;
  /// number of uniform uncertain variables (from the \c uniform_uncertain
  /// specification in \ref VarAUV)
  size_t numUniformUncVars;
  /// number of loguniform uncertain variables (from the \c
  /// loguniform_uncertain specification in \ref VarAUV)
  size_t numLoguniformUncVars;
  /// number of triangular uncertain variables (from the \c
  /// triangular_uncertain specification in \ref VarAUV)
  size_t numTriangularUncVars;
  /// number of exponential uncertain variables (from the \c
  /// exponential_uncertain specification in \ref VarAUV)
  size_t numExponentialUncVars;
  /// number of beta uncertain variables (from the \c beta_uncertain
  /// specification in \ref VarAUV)
  size_t numBetaUncVars;
  /// number of gamma uncertain variables (from the \c gamma_uncertain
  /// specification in \ref VarAUV)
  size_t numGammaUncVars;
  /// number of gumbel uncertain variables (from the \c gumbel_uncertain
  /// specification in \ref VarAUV)
  size_t numGumbelUncVars;
  /// number of frechet uncertain variables (from the \c frechet_uncertain
  /// specification in \ref VarAUV)
  size_t numFrechetUncVars;
  /// number of weibull uncertain variables (from the \c weibull_uncertain
  /// specification in \ref VarAUV)
  size_t numWeibullUncVars;
  /// number of histogram bin uncertain variables (from the \c
  /// histogram_bin_uncertain specification in \ref VarAUV)
  size_t numHistogramBinUncVars;
  /// number of Poisson uncertain variables (from the \c
  /// poisson_uncertain specification in \ref VarAUV)
  size_t numPoissonUncVars;
  /// number of binomial uncertain variables (from the \c
  /// binomial_uncertain specification in \ref VarAUV)
  size_t numBinomialUncVars;
  /// number of negative binomial uncertain variables (from the \c
  /// negative_binomial_uncertain specification in \ref VarAUV)
  size_t numNegBinomialUncVars;
  /// number of geometric uncertain variables (from the \c
  /// geometric_uncertain specification in \ref VarAUV
  size_t numGeometricUncVars;
  /// number of hypergeometric uncertain variables (from the \c
  /// hypergeometric_uncertain specification in \ref VarAUV))
  size_t numHyperGeomUncVars;
  /// number of histogram point uncertain variables (from the \c
  /// histogram_point_uncertain specification in \ref VarAUV)
  size_t numHistogramPtUncVars;
  /// number of continuous epistemic interval uncertain variables (from
  /// the \c continuous_interval_uncertain specification in \ref VarEUV)
  size_t numContinuousIntervalUncVars;
  /// number of discrete epistemic interval uncertain variables (from the
  /// \c discrete_interval_uncertain specification in \ref VarEUV)
  size_t numDiscreteIntervalUncVars;
  /// number of discrete epistemic uncertain integer set variables (from
  /// the \c discrete_uncertain_set_integer specification in \ref VarEUV)
  size_t numDiscreteUncSetIntVars;
  /// number of discrete epistemic uncertain real set variables (from
  /// the \c discrete_uncertain_set_real specification in \ref VarEUV)
  size_t numDiscreteUncSetRealVars;
  /// number of continuous state variables (from the \c continuous_state
  /// specification in \ref VarSV)
  size_t numContinuousStateVars;
  /// number of discrete state variables defined by an integer range
  /// (from the \c discrete_state_range specification in \ref VarDV)
  size_t numDiscreteStateRangeVars;
  /// number of discrete state variables defined by a set of integers
  /// (from the \c discrete_state_set_integer specification in \ref VarDV)
  size_t numDiscreteStateSetIntVars;
  /// number of discrete state variables defined by a set of reals
  /// (from the \c discrete_state_set_real specification in \ref VarDV)
  size_t numDiscreteStateSetRealVars;

  // Design arrays

  /// initial values for the continuous design variables array (from
  /// the \c continuous_design \c initial_point specification in \ref VarDV)
  RealVector continuousDesignVars;
  /// lower bounds array for the continuous design variables (from the 
  /// \c continuous_design \c lower_bounds specification in \ref VarDV)
  RealVector continuousDesignLowerBnds;
  /// upper bounds array for the continuous design variables (from the
  /// \c continuous_design \c upper_bounds specification in \ref VarDV)
  RealVector continuousDesignUpperBnds;
  /// scale types array for the continuous design variables (from the
  /// \c continuous_design \c scale_types specification in \ref VarDV)
  StringArray continuousDesignScaleTypes;
  /// scales array for the continuous design variables (from the
  /// \c continuous_design \c scales specification in \ref VarDV)
  RealVector continuousDesignScales;
  /// initial values for the discrete design variables defined by an
  /// integer range (from the \c discrete_design_range \c initial_point
  /// specification in \ref VarDV)
  IntVector discreteDesignRangeVars;
  /// lower bounds array for the discrete design variables defined by an
  /// integer range (from the \c discrete_design_range \c lower_bounds
  /// specification in \ref VarDV)
  IntVector discreteDesignRangeLowerBnds;
  /// upper bounds array for the discrete design variables defined
  /// by an integer range(from the \c discrete_design_range \c
  /// upper_bounds specification in \ref VarDV)
  IntVector discreteDesignRangeUpperBnds;
  /// initial values for the discrete design variables defined by an
  /// integer set (from the \c discrete_design_set_integer \c initial_point
  /// specification in \ref VarDV)
  IntVector discreteDesignSetIntVars;
  /// initial values for the discrete design variables defined by a
  /// real set (from the \c discrete_design_set_real \c initial_point
  /// specification in \ref VarDV)
  RealVector discreteDesignSetRealVars;
  /// complete set of admissible values for each of the discrete design
  /// variables defined by an integer set (from the
  /// \c discrete_design_set_integer \c set_values specification in \ref VarDV)
  IntSetArray discreteDesignSetInt;
  /// complete set of admissible values for each of the discrete design
  /// variables defined by a real set (from the \c discrete_design_set_real
  /// \c set_values specification in \ref VarDV)
  RealSetArray discreteDesignSetReal;
  /// labels array for the continuous design variables (from the
  /// \c continuous_design \c descriptors specification in \ref VarDV)
  StringArray continuousDesignLabels;
  /// labels array for the discrete design variables defined by an
  /// integer range (from the \c discrete_design_range \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignRangeLabels;
  /// labels array for the discrete design variables defined by an
  /// integer set (from the \c discrete_design_range \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignSetIntLabels;
  /// labels array for the discrete design variables defined by a
  /// real set (from the \c discrete_design_range \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignSetRealLabels;

  // Aleatory uncertain arrays

  /// means of the normal uncertain variables (from the \c nuv_means
  /// specification in \ref VarAUV)
  RealVector normalUncMeans;
  /// standard deviations of the normal uncertain variables (from
  /// the \c nuv_std_deviations specification in \ref VarAUV)
  RealVector normalUncStdDevs;
  /// distribution lower bounds for the normal uncertain variables
  /// (from the \c nuv_lower_bounds specification in \ref VarAUV)
  RealVector normalUncLowerBnds;
  /// distribution upper bounds for the normal uncertain variables
  /// (from the \c nuv_upper_bounds specification in \ref VarAUV)
  RealVector normalUncUpperBnds;
  /// lambdas (means of the corresponding normals) of the lognormal uncertain
  /// variables (from the \c lnuv_lambdas specification in \ref VarAUV)
  RealVector lognormalUncLambdas;
  /// zetas (standard deviations of the corresponding normals) of the lognormal
  /// uncertain variables (from the \c lnuv_zetas specification in \ref VarAUV)
  RealVector lognormalUncZetas;
  /// means of the lognormal uncertain variables (from the \c
  /// lnuv_means specification in \ref VarAUV)
  RealVector lognormalUncMeans;
  /// standard deviations of the lognormal uncertain variables (from
  /// the \c lnuv_std_deviations specification in \ref VarAUV)
  RealVector lognormalUncStdDevs;
  /// error factors for the lognormal uncertain variables (from
  /// the \c lnuv_error_factors specification in \ref VarAUV)
  RealVector lognormalUncErrFacts;
  /// distribution lower bounds for the lognormal uncertain variables
  /// (from the \c lnuv_lower_bounds specification in \ref VarAUV)
  RealVector lognormalUncLowerBnds;
  /// distribution upper bounds for the lognormal uncertain variables
  /// (from the \c lnuv_upper_bounds specification in \ref VarAUV)
  RealVector lognormalUncUpperBnds;
  /// distribution lower bounds for the uniform uncertain variables
  /// (from the \c uuv_lower_bounds specification in \ref VarAUV)
  RealVector uniformUncLowerBnds;
  /// distribution upper bounds for the uniform uncertain variables
  /// (from the \c uuv_upper_bounds specification in \ref VarAUV)
  RealVector uniformUncUpperBnds;
  /// distribution lower bounds for the loguniform uncertain variables
  /// (from the \c luuv_lower_bounds specification in \ref VarAUV)
  RealVector loguniformUncLowerBnds;
  /// distribution upper bounds for the loguniform uncertain variables
  /// (from the \c luuv_upper_bounds specification in \ref VarAUV)
  RealVector loguniformUncUpperBnds;
  /// modes of the triangular uncertain variables (from the \c tuv_modes
  /// specification in \ref VarAUV)
  RealVector triangularUncModes;
  /// distribution lower bounds for the triangular uncertain variables
  /// (from the \c tuv_lower_bounds specification in \ref VarAUV)
  RealVector triangularUncLowerBnds;
  /// distribution upper bounds for the triangular uncertain variables
  /// (from the \c tuv_upper_bounds specification in \ref VarAUV)
  RealVector triangularUncUpperBnds;
  /// beta factors for the exponential uncertain variables (from
  /// the \c euv_betas specification in \ref VarAUV)
  RealVector exponentialUncBetas;
  /// alpha factors for the beta uncertain variables (from
  /// the \c buv_means specification in \ref VarAUV)
  RealVector betaUncAlphas;
  /// beta factors for the beta uncertain variables (from
  /// the \c buv_std_deviations specification in \ref VarAUV)
  RealVector betaUncBetas;
  /// distribution lower bounds for the beta uncertain variables
  /// (from the \c buv_lower_bounds specification in \ref VarAUV)
  RealVector betaUncLowerBnds;
  /// distribution upper bounds for the beta uncertain variables
  /// (from the \c buv_upper_bounds specification in \ref VarAUV)
  RealVector betaUncUpperBnds;
  /// alpha factors for the gamma uncertain variables (from
  /// the \c gauv_alphas specification in \ref VarAUV)
  RealVector gammaUncAlphas;
  /// beta factors for the gamma uncertain variables (from
  /// the \c gauv_betas specification in \ref VarAUV)
  RealVector gammaUncBetas;
  /// alpha factors for the gumbel uncertain variables (from the \c
  /// guuv_alphas specification in \ref VarAUV)
  RealVector gumbelUncAlphas;
  /// beta factors for of the gumbel uncertain variables (from
  /// the \c guuv_betas specification in \ref VarAUV)
  RealVector gumbelUncBetas;
  /// alpha factors for the frechet uncertain variables (from
  /// the \c fuv_alphas specification in \ref VarAUV)
  RealVector frechetUncAlphas;
  /// beta factors for the frechet uncertain variables (from
  /// the \c fuv_betas specification in \ref VarAUV)
  RealVector frechetUncBetas;
  /// alpha factors for the weibull uncertain variables (from
  /// the \c wuv_alphas specification in \ref VarAUV)
  RealVector weibullUncAlphas;
  /// beta factors for the weibull uncertain variables (from
  /// the \c wuv_betas specification in \ref VarAUV)
  RealVector weibullUncBetas;
  /// an array containing a vector of (x,c) pairs for each bin-based
  /// histogram uncertain variable (see continuous linear histogram in
  /// LHS manual; from the \c histogram_bin_uncertain specification in
  /// \ref VarAUV).  (x,y) ordinate specifications are converted to (x,c)
  /// counts within NIDR.
  RealVectorArray histogramUncBinPairs;
  /// lambdas (rate parameter) for the poisson uncertain variables (from
  /// the \c lambdas specification in \ref VarAUV)
  RealVector poissonUncLambdas;
  /// probabilities per each trial (p) for the binomial uncertain variables
  /// from the \c prob_per_trial specification in \ref VarAUV)
  RealVector binomialUncProbPerTrial;
  /// Number of trials (N) for the binomial uncertain variables
  /// from the \c num_trials specification in \ref VarAUV)
  IntVector binomialUncNumTrials;
  /// probabilities per each trial (p) for the negative binomial uncertain 
  /// variables from the \c prob_per_trial specification in \ref VarAUV)
  RealVector negBinomialUncProbPerTrial;
  /// Number of trials (N) for the negative binomial uncertain variables
  /// from the \c num_trials specification in \ref VarAUV)
  IntVector negBinomialUncNumTrials;
  /// probabilities per each trial (p) for the geometric uncertain 
  /// variables from the \c prob_per_trial specification in \ref VarAUV)
  RealVector geometricUncProbPerTrial;
  /// Size of total populations (N) for the hypergeometric uncertain variables
  /// from the \c total_population specification in \ref VarAUV)
  IntVector hyperGeomUncTotalPop;
  /// Size of selected populations for the hypergeometric uncertain variables
  /// from the \c selected_population specification in \ref VarAUV)
  IntVector hyperGeomUncSelectedPop;
  /// Number failed in the selected populations for the hypergeometric 
  /// variablesfrom the \c num_drawn specification in \ref VarAUV)
  IntVector hyperGeomUncNumDrawn;
  /// an array containing a vector of (x,c) pairs for each point-based
  /// histogram uncertain variable (see discrete histogram in LHS manual;
  /// from the \c histogram_point_uncertain specification in \ref VarAUV)
  RealVectorArray histogramUncPointPairs;
  /// correlation matrix for all uncertain variables (from the \c
  /// uncertain_correlation_matrix specification in \ref VarAUV). This
  /// matrix specifies rank correlations for sampling methods (i.e.,
  /// LHS) and correlation coefficients (rho_ij = normalized covariance
  /// matrix) for analytic reliability methods.
  RealSymMatrix uncertainCorrelations;

  // Epistemic uncertain arrays

  /// Probability values per interval cell per epistemic interval uncertain
  /// variable (from the \c continuous_interval_uncertain \c interval_probs
  /// specification in \ref VarEUV)
  RealVectorArray continuousIntervalUncBasicProbs;
  /// lower bounds defining cells for each epistemic interval uncertain
  /// variable (from the \c continuous_interval_uncertain \c lower_bounds
  /// specification in \ref VarEUV)
  RealVectorArray continuousIntervalUncLowerBounds;
  /// upper bounds defining cells for each epistemic interval uncertain
  /// variable (from the \c continuous_interval_uncertain \c upper_bounds
  /// specification in \ref VarEUV)
  RealVectorArray continuousIntervalUncUpperBounds;
  /// Probability values per interval cell per epistemic interval uncertain
  /// variable (from the \c discrete_interval_uncertain \c interval_probs
  /// specification in \ref VarEUV)
  RealVectorArray discreteIntervalUncBasicProbs;
  /// lower bounds defining cells for each epistemic interval uncertain
  /// variable (from the \c discrete_interval_uncertain \c lower_bounds
  /// specification in \ref VarEUV)
  IntVectorArray discreteIntervalUncLowerBounds;
  /// upper bounds defining cells for each epistemic interval uncertain
  /// variable (from the \c discrete_interval_uncertain \c upper_bounds
  /// specification in \ref VarEUV)
  IntVectorArray discreteIntervalUncUpperBounds;
  /// complete set of admissible values with associated basic probability
  /// assignments for each of the discrete epistemic uncertain variables
  /// defined by an integer set (from the \c discrete_uncertain_set_integer
  /// \c set_values specification in \ref VarEUV)
  IntRealMapArray discreteUncSetIntValuesProbs;
  /// complete set of admissible values with associated basic probability
  /// assignments for each of the discrete epistemic uncertain variables
  /// defined by a real set (from the \c discrete_uncertain_set_real
  /// \c set_values specification in \ref VarEUV)
  RealRealMapArray discreteUncSetRealValuesProbs;

  // State arrays

  /// initial values for the continuous state variables array (from
  /// the \c continuous_state \c initial_point specification in \ref VarSV)
  RealVector continuousStateVars;
  /// lower bounds array for the continuous state variables (from the 
  /// \c continuous_state \c lower_bounds specification in \ref VarSV)
  RealVector continuousStateLowerBnds;
  /// upper bounds array for the continuous state variables (from the
  /// \c continuous_state \c upper_bounds specification in \ref VarSV)
  RealVector continuousStateUpperBnds;
  /// initial values for the discrete state variables defined by an
  /// integer range (from the \c discrete_state_range \c initial_point
  /// specification in \ref VarSV)
  IntVector discreteStateRangeVars;
  /// lower bounds array for the discrete state variables defined by an
  /// integer range (from the \c discrete_state_range \c lower_bounds
  /// specification in \ref VarSV)
  IntVector discreteStateRangeLowerBnds;
  /// upper bounds array for the discrete state variables defined
  /// by an integer range(from the \c discrete_state_range \c
  /// upper_bounds specification in \ref VarSV)
  IntVector discreteStateRangeUpperBnds;
  /// initial values for the discrete state variables defined by an
  /// integer set (from the \c discrete_state_set_integer \c initial_point
  /// specification in \ref VarSV)
  IntVector discreteStateSetIntVars;
  /// initial values for the discrete state variables defined by a
  /// real set (from the \c discrete_state_set_real \c initial_point
  /// specification in \ref VarSV)
  RealVector discreteStateSetRealVars;
  /// complete set of admissible values for each of the discrete state
  /// variables defined by an integer set (from the
  /// \c discrete_state_set_integer \c set_values specification in \ref VarSV)
  IntSetArray discreteStateSetInt;
  /// complete set of admissible values for each of the discrete state
  /// variables defined by a real set (from the \c discrete_state_set_real
  /// \c set_values specification in \ref VarSV)
  RealSetArray discreteStateSetReal;
  /// labels array for the continuous state variables (from the
  /// \c continuous_state \c descriptors specification in \ref VarSV)
  StringArray continuousStateLabels;
  /// labels array for the discrete state variables defined by an
  /// integer range (from the \c discrete_state_range \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateRangeLabels;
  /// labels array for the discrete state variables defined by an
  /// integer set (from the \c discrete_state_range \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateSetIntLabels;
  /// labels array for the discrete state variables defined by a
  /// real set (from the \c discrete_state_range \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateSetRealLabels;

  // Inferred arrays

  /// discrete design integer set lower bounds inferred from set values
  IntVector discreteDesignSetIntLowerBnds;
  /// discrete design integer set upper bounds inferred from set values
  IntVector discreteDesignSetIntUpperBnds;
  /// discrete design real set lower bounds inferred from set values
  RealVector discreteDesignSetRealLowerBnds;
  /// discrete design real set upper bounds inferred from set values
  RealVector discreteDesignSetRealUpperBnds;
  /// array of values for all continuous aleatory uncertain variables
  RealVector continuousAleatoryUncVars;
  /// distribution lower bounds for all continuous aleatory uncertain variables
  /// (collected from \c nuv_lower_bounds, \c lnuv_lower_bounds,
  /// \c uuv_lower_bounds, \c luuv_lower_bounds, \c tuv_lower_bounds,
  /// and \c buv_lower_bounds specifications in \ref VarAUV, and derived
  /// for gamma, gumbel, frechet, weibull and histogram bin specifications)
  RealVector continuousAleatoryUncLowerBnds;
  /// distribution upper bounds for all continuous aleatory uncertain variables
  /// (collected from \c nuv_upper_bounds, \c lnuv_upper_bounds,
  /// \c uuv_upper_bounds, \c luuv_upper_bounds, \c tuv_lower_bounds,
  /// and \c buv_upper_bounds specifications in \ref VarAUV, and derived
  /// for gamma, gumbel, frechet, weibull and histogram bin specifications)
  RealVector continuousAleatoryUncUpperBnds;
  /// labels for all continuous aleatory uncertain variables
  /// (collected from \c nuv_descriptors, \c lnuv_descriptors, \c
  /// uuv_descriptors, \c luuv_descriptors, \c tuv_descriptors, \c
  /// buv_descriptors, \c gauv_descriptors, \c guuv_descriptors, \c
  /// fuv_descriptors, \c wuv_descriptors, and \c hbuv_descriptors
  /// specifications in \ref VarAUV)
  StringArray continuousAleatoryUncLabels;
  /// array of values for all discrete integer aleatory uncertain variables
  IntVector discreteIntAleatoryUncVars;
  /// distribution lower bounds for all discrete integer aleatory
  /// uncertain variables
  IntVector discreteIntAleatoryUncLowerBnds;
  /// distribution upper bounds for all discrete integer aleatory
  /// uncertain variables
  IntVector discreteIntAleatoryUncUpperBnds;
  /// labels for all discrete integer aleatory uncertain variables
  StringArray discreteIntAleatoryUncLabels;
  /// array of values for all discrete real aleatory uncertain variables
  RealVector discreteRealAleatoryUncVars;
  /// distribution lower bounds for all discrete real aleatory
  /// uncertain variables
  RealVector discreteRealAleatoryUncLowerBnds;
  /// distribution upper bounds for all discrete real aleatory
  /// uncertain variables
  RealVector discreteRealAleatoryUncUpperBnds;
  /// labels for all discrete real aleatory uncertain variables
  StringArray discreteRealAleatoryUncLabels;
  /// array of values for all continuous epistemic uncertain variables
  RealVector continuousEpistemicUncVars;
  /// distribution lower bounds for all continuous epistemic uncertain variables
  RealVector continuousEpistemicUncLowerBnds;
  /// distribution upper bounds for all continuous epistemic uncertain variables
  RealVector continuousEpistemicUncUpperBnds;
  /// labels for all continuous epistemic uncertain variables
  StringArray continuousEpistemicUncLabels;
  /// array of values for all discrete integer epistemic uncertain variables
  IntVector discreteIntEpistemicUncVars;
  /// distribution lower bounds for all discrete integer epistemic
  /// uncertain variables
  IntVector discreteIntEpistemicUncLowerBnds;
  /// distribution upper bounds for all discrete integer epistemic
  /// uncertain variables
  IntVector discreteIntEpistemicUncUpperBnds;
  /// labels for all discrete integer epistemic uncertain variables
  StringArray discreteIntEpistemicUncLabels;
  /// array of values for all discrete real epistemic uncertain variables
  RealVector discreteRealEpistemicUncVars;
  /// distribution lower bounds for all discrete real epistemic
  /// uncertain variables
  RealVector discreteRealEpistemicUncLowerBnds;
  /// distribution upper bounds for all discrete real epistemic
  /// uncertain variables
  RealVector discreteRealEpistemicUncUpperBnds;
  /// labels for all discrete real epistemic uncertain variables
  StringArray discreteRealEpistemicUncLabels;
  /// discrete state integer set lower bounds inferred from set values
  IntVector discreteStateSetIntLowerBnds;
  /// discrete state integer set upper bounds inferred from set values
  IntVector discreteStateSetIntUpperBnds;
  /// discrete state real set lower bounds inferred from set values
  RealVector discreteStateSetRealLowerBnds;
  /// discrete state real set upper bounds inferred from set values
  RealVector discreteStateSetRealUpperBnds;

private:

  //
  //- Heading: Private functions
  //

  DataVariablesRep();  ///< default constructor
  ~DataVariablesRep(); ///< destructor

  /// write a DataVariablesRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataVariablesRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataVariablesRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing dataVarsRep
  int referenceCount;
};


inline DataVariablesRep::~DataVariablesRep()
{ }


/// Handle class for variables specification data.

/** The DataVariables class is used to provide a memory management
    handle for the data in DataVariablesRep.  It is populated by
    IDRProblemDescDB::variables_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A list of
    DataVariables objects is maintained in
    ProblemDescDB::dataVariablesList, one for each variables
    specification in an input file. */

class DataVariables
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;
  /// library_mode default data initializer
  friend void run_dakota_data();

public:

  /// compares the idVariables attribute of DataVariables objects
  static bool id_compare(const DataVariables& dv, const std::string& id)
  { return id == dv.dataVarsRep->idVariables; }

  //
  //- Heading: Constructors, destructor, operators
  //

  DataVariables();                                ///< constructor
  DataVariables(const DataVariables&);            ///< copy constructor
  ~DataVariables();                               ///< destructor

  DataVariables operator=(const DataVariables&); ///< assignment operator
  bool operator==(const DataVariables&);          ///< equality operator

  //
  //- Heading: Member methods
  //

  /// write a DataVariables object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataVariables object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataVariables object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  // Get Functions (composite variable counts only):

  /// return total number of design variables
  size_t design() { return dataVarsRep->numContinuousDesVars +
    dataVarsRep->numDiscreteDesRangeVars +
    dataVarsRep->numDiscreteDesSetIntVars +
    dataVarsRep->numDiscreteDesSetRealVars; }
  /// return total number of aleatory uncertain variables
  size_t aleatory_uncertain() { return dataVarsRep->numNormalUncVars +
    dataVarsRep->numLognormalUncVars + dataVarsRep->numUniformUncVars +
    dataVarsRep->numLoguniformUncVars + dataVarsRep->numTriangularUncVars +
    dataVarsRep->numExponentialUncVars + dataVarsRep->numBetaUncVars +
    dataVarsRep->numGammaUncVars + dataVarsRep->numGumbelUncVars +
    dataVarsRep->numFrechetUncVars + dataVarsRep->numWeibullUncVars +
    dataVarsRep->numHistogramBinUncVars + dataVarsRep->numPoissonUncVars + 
    dataVarsRep->numBinomialUncVars + dataVarsRep->numNegBinomialUncVars+
    dataVarsRep->numGeometricUncVars + dataVarsRep->numHyperGeomUncVars +
    dataVarsRep->numHistogramPtUncVars; }
  /// return total number of epistemic uncertain variables
  size_t epistemic_uncertain() { return
    dataVarsRep->numContinuousIntervalUncVars +
    dataVarsRep->numDiscreteIntervalUncVars +
    dataVarsRep->numDiscreteUncSetIntVars +
    dataVarsRep->numDiscreteUncSetRealVars; }
  /// return total number of uncertain variables
  size_t uncertain() { return aleatory_uncertain() + epistemic_uncertain(); }
  /// return total number of state variables
  size_t state() { return dataVarsRep->numContinuousStateVars +
    dataVarsRep->numDiscreteStateRangeVars +
    dataVarsRep->numDiscreteStateSetIntVars +
    dataVarsRep->numDiscreteStateSetRealVars; }
  /// return total number of continuous variables
  size_t continuous_variables() { return dataVarsRep->numContinuousDesVars +
    uncertain() + dataVarsRep->numContinuousStateVars; }
  /// return total number of discrete variables
  size_t discrete_variables()   { return dataVarsRep->numDiscreteDesRangeVars +
    dataVarsRep->numDiscreteDesSetIntVars +
    dataVarsRep->numDiscreteDesSetRealVars +
    dataVarsRep->numDiscreteStateRangeVars +
    dataVarsRep->numDiscreteStateSetIntVars +
    dataVarsRep->numDiscreteStateSetRealVars; }
  /// return total number of variables
  size_t total_variables() { return design() + uncertain() + state(); }

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataVariablesRep* dataVarsRep;
};


inline bool DataVariables::operator==(const DataVariables& data_vars)
{
  Cerr << "Warning:  This is not a full equality operator for the "
       << "DataVariables class" << std::endl;
  return (data_vars.dataVarsRep->idVariables == dataVarsRep->idVariables);
}



/// MPIPackBuffer insertion operator for DataVariables
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataVariables& data)
{ data.write(s); return s;}


/// MPIUnpackBuffer extraction operator for DataVariables
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataVariables& data)
{ data.read(s); return s;}


/// std::ostream insertion operator for DataVariables
inline std::ostream& operator<<(std::ostream& s, const DataVariables& data)
{ data.write(s); return s;}


inline void DataVariables::write(std::ostream& s) const
{ dataVarsRep->write(s); }


inline void DataVariables::read(MPIUnpackBuffer& s)
{ dataVarsRep->read(s); }


inline void DataVariables::write(MPIPackBuffer& s) const
{ dataVarsRep->write(s); }

} // namespace Dakota

#endif
