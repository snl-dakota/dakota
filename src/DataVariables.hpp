/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataVariables
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataVariables.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DATA_VARIABLES_H
#define DATA_VARIABLES_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"  // for Cerr
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"

namespace Dakota {

// special values for domain: Relaxed or Mixed
enum { DEFAULT_DOMAIN=0, RELAXED_DOMAIN, MIXED_DOMAIN };
// special values for active view: All, Design, {A+E,A,E} Uncertain, or State
enum { DEFAULT_VIEW=0, ALL_VIEW, DESIGN_VIEW, UNCERTAIN_VIEW,
       ALEATORY_UNCERTAIN_VIEW, EPISTEMIC_UNCERTAIN_VIEW, STATE_VIEW };
// special values for active and inactive views include EMPTY_VIEW plus the
// 12 combinations of All, Design, {A+E,A,E} Uncertain, or State active view
// [6 total] and Relaxed or Mixed domain [2 total].
enum { EMPTY_VIEW=0, RELAXED_ALL, MIXED_ALL,
       RELAXED_DESIGN, RELAXED_UNCERTAIN, RELAXED_ALEATORY_UNCERTAIN,
       RELAXED_EPISTEMIC_UNCERTAIN, RELAXED_STATE,
       MIXED_DESIGN, MIXED_UNCERTAIN, MIXED_ALEATORY_UNCERTAIN,
       MIXED_EPISTEMIC_UNCERTAIN, MIXED_STATE };

/// values differentiating subsets of variables for I/O
enum { ALL_VARS=0, ACTIVE_VARS, INACTIVE_VARS};

// special values for SharedVariablesData::variablesComponents and
// SharedVariablesData::all*Types (Note: these differ from the
// pecos_global_defs.hpp enumeration, which is focused on ranVarTypesX/U)
enum { EMPTY_TYPE=0, CONTINUOUS_DESIGN, DISCRETE_DESIGN_RANGE,
       DISCRETE_DESIGN_SET_INT, DISCRETE_DESIGN_SET_STRING,
       DISCRETE_DESIGN_SET_REAL, NORMAL_UNCERTAIN, LOGNORMAL_UNCERTAIN,
       UNIFORM_UNCERTAIN, LOGUNIFORM_UNCERTAIN, TRIANGULAR_UNCERTAIN,
       EXPONENTIAL_UNCERTAIN, BETA_UNCERTAIN, GAMMA_UNCERTAIN, GUMBEL_UNCERTAIN,
       FRECHET_UNCERTAIN, WEIBULL_UNCERTAIN, HISTOGRAM_BIN_UNCERTAIN,
       POISSON_UNCERTAIN, BINOMIAL_UNCERTAIN, NEGATIVE_BINOMIAL_UNCERTAIN,
       GEOMETRIC_UNCERTAIN, HYPERGEOMETRIC_UNCERTAIN,
       HISTOGRAM_POINT_UNCERTAIN_INT, HISTOGRAM_POINT_UNCERTAIN_STRING,
       HISTOGRAM_POINT_UNCERTAIN_REAL, CONTINUOUS_INTERVAL_UNCERTAIN,
       DISCRETE_INTERVAL_UNCERTAIN, DISCRETE_UNCERTAIN_SET_INT,
       DISCRETE_UNCERTAIN_SET_STRING, DISCRETE_UNCERTAIN_SET_REAL,
       CONTINUOUS_STATE, DISCRETE_STATE_RANGE, DISCRETE_STATE_SET_INT,
       DISCRETE_STATE_SET_STRING, DISCRETE_STATE_SET_REAL };

// special values for indexing of SharedVariablesData::variablesCompsTotals;
// last value is used for sizing arrays of vars_comps_totals
enum { TOTAL_CDV=0, TOTAL_DDIV,  TOTAL_DDSV,  TOTAL_DDRV,
       TOTAL_CAUV,  TOTAL_DAUIV, TOTAL_DAUSV, TOTAL_DAURV,
       TOTAL_CEUV,  TOTAL_DEUIV, TOTAL_DEUSV, TOTAL_DEURV,
       TOTAL_CSV,   TOTAL_DSIV,  TOTAL_DSSV,  TOTAL_DSRV,
       NUM_VC_TOTALS};


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
  /// flag indicating user specification of initial points (for local
  /// optimization-based UQ methods) for at least one uncertain variable type
  bool uncertainVarsInitPt;
  /// number of continuous design variables (from the \c continuous_design
  /// specification in \ref VarDV)
  size_t numContinuousDesVars;
  /// number of discrete design variables defined by an integer range
  /// (from the \c discrete_design_range specification in \ref VarDV)
  size_t numDiscreteDesRangeVars;
  /// number of discrete design variables defined by a set of integers
  /// (from the \c discrete_design_set integer specification in \ref VarDV)
  size_t numDiscreteDesSetIntVars;
  /// number of discrete design variables defined by a set of strings
  /// (from the \c discrete_design_set string specification in \ref VarDV)
  size_t numDiscreteDesSetStrVars;
  /// number of discrete design variables defined by a set of reals
  /// (from the \c discrete_design_set real specification in \ref VarDV)
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
  /// number of integer-valued histogram point uncertain variables (from the \c
  /// histogram_point_uncertain specification in \ref VarAUV)
  size_t numHistogramPtIntUncVars;
  /// number of string-valued histogram point uncertain variables (from the \c
  /// histogram_point_uncertain specification in \ref VarAUV)
  size_t numHistogramPtStrUncVars;
  /// number of real-valued histogram point uncertain variables (from the \c
  /// histogram_point_uncertain specification in \ref VarAUV)
  size_t numHistogramPtRealUncVars;
  /// number of continuous epistemic interval uncertain variables (from
  /// the \c continuous_interval_uncertain specification in \ref VarEUV)
  size_t numContinuousIntervalUncVars;
  /// number of discrete epistemic interval uncertain variables (from the
  /// \c discrete_interval_uncertain specification in \ref VarEUV)
  size_t numDiscreteIntervalUncVars;
  /// number of discrete epistemic uncertain integer set variables (from
  /// the \c discrete_uncertain_set integer specification in \ref VarEUV)
  size_t numDiscreteUncSetIntVars;
  /// number of discrete epistemic uncertain string set variables (from
  /// the \c discrete_uncertain_set string specification in \ref VarEUV)
  size_t numDiscreteUncSetStrVars;
  /// number of discrete epistemic uncertain real set variables (from
  /// the \c discrete_uncertain_set real specification in \ref VarEUV)
  size_t numDiscreteUncSetRealVars;
  /// number of continuous state variables (from the \c continuous_state
  /// specification in \ref VarSV)
  size_t numContinuousStateVars;
  /// number of discrete state variables defined by an integer range
  /// (from the \c discrete_state_range specification in \ref VarDV)
  size_t numDiscreteStateRangeVars;
  /// number of discrete state variables defined by a set of integers
  /// (from the \c discrete_state_set integer specification in \ref VarDV)
  size_t numDiscreteStateSetIntVars;
  /// number of discrete state variables defined by a set of strings
  /// (from the \c discrete_state_set string specification in \ref VarDV)
  size_t numDiscreteStateSetStrVars;
  /// number of discrete state variables defined by a set of reals
  /// (from the \c discrete_state_set real specification in \ref VarDV)
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
  /// is each ddr var strictly categorical (true) or relaxable (false)
  BitArray discreteDesignRangeCat;

  /// initial values for the discrete design variables defined by an
  /// integer set (from the \c discrete_design_set integer \c initial_point
  /// specification in \ref VarDV)
  IntVector discreteDesignSetIntVars;
  /// initial values for the discrete design variables defined by a
  /// string set (from the \c discrete_design_set string \c initial_point
  /// specification in \ref VarDV)
  StringArray discreteDesignSetStrVars;
  /// initial values for the discrete design variables defined by a
  /// real set (from the \c discrete_design_set real \c initial_point
  /// specification in \ref VarDV)
  RealVector discreteDesignSetRealVars;
  /// complete set of admissible values for each of the discrete design
  /// variables defined by an integer set (from the
  /// \c discrete_design_set integer \c set_values specification in \ref VarDV)
  IntSetArray discreteDesignSetInt;
  /// complete set of admissible values for each of the discrete design
  /// variables defined by a string set (from the
  /// \c discrete_design_set string \c set_values specification in \ref VarDV)
  StringSetArray discreteDesignSetStr;
  /// complete set of admissible values for each of the discrete design
  /// variables defined by a real set (from the \c discrete_design_set real
  /// \c set_values specification in \ref VarDV)
  RealSetArray discreteDesignSetReal;

  /// is each ddsi var strictly categorical (true) or relaxable (false)
  BitArray discreteDesignSetIntCat;
  /// is each ddsr var strictly categorical (true) or relaxable (false)
  BitArray discreteDesignSetRealCat;
  
  /// Adjacency matrices for each of the discrete design variables defined
  /// by an integer set (from the \c discrete_design_set integer
  /// \c categorical \c adjacency specification in \ref VarDV
  RealMatrixArray discreteDesignSetIntAdj;
  /// Adjacency matrices for each of the discrete design variables defined
  /// by a string set (from the \c discrete_design_set string
  ///  \c adjacency specification in \ref VarDV
  RealMatrixArray discreteDesignSetStrAdj;
  /// Adjacency matrices for each of the discrete design variables defined
  /// by a real set (from the \c discrete_design_set real
  /// \c categorical \c adjacency specification in \ref VarDV
  RealMatrixArray discreteDesignSetRealAdj;

  /// labels array for the continuous design variables (from the
  /// \c continuous_design \c descriptors specification in \ref VarDV)
  StringArray continuousDesignLabels;
  /// labels array for the discrete design variables defined by an
  /// integer range (from the \c discrete_design_range \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignRangeLabels;
  /// labels array for the discrete design variables defined by an
  /// integer set (from the \c discrete_design set int \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignSetIntLabels;
  /// labels array for the discrete design variables defined by a
  /// string set (from the \c discrete_design_set string \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignSetStrLabels;
  /// labels array for the discrete design variables defined by a
  /// real set (from the \c discrete_design_set real; \c descriptors
  /// specification in \ref VarDV)
  StringArray discreteDesignSetRealLabels;

  // Aleatory uncertain arrays

  /// means of the normal uncertain variables (from the \c means
  /// specification in \ref VarCAUV_Normal)
  RealVector normalUncMeans;
  /// standard deviations of the normal uncertain variables (from
  /// the \c std_deviations specification in \ref VarCAUV_Normal)
  RealVector normalUncStdDevs;
  /// distribution lower bounds for the normal uncertain variables
  /// (from the \c lower_bounds specification in \ref VarCAUV_Normal)
  RealVector normalUncLowerBnds;
  /// distribution upper bounds for the normal uncertain variables
  /// (from the \c upper_bounds specification in \ref VarCAUV_Normal)
  RealVector normalUncUpperBnds;
  /// initial values of the normal uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Normal)
  RealVector normalUncVars;
  /// lambdas (means of the corresponding normals) of the lognormal uncertain
  /// variables (from the \c lambdas specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncLambdas;
  /// zetas (standard deviations of the corresponding normals) of the
  /// lognormal uncertain variables (from the \c zetas specification
  /// in \ref VarCAUV_Lognormal)
  RealVector lognormalUncZetas;
  /// means of the lognormal uncertain variables (from the \c
  /// means specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncMeans;
  /// standard deviations of the lognormal uncertain variables (from
  /// the \c std_deviations specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncStdDevs;
  /// error factors for the lognormal uncertain variables (from
  /// the \c error_factors specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncErrFacts;
  /// distribution lower bounds for the lognormal uncertain variables
  /// (from the \c lower_bounds specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncLowerBnds;
  /// distribution upper bounds for the lognormal uncertain variables
  /// (from the \c upper_bounds specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncUpperBnds;
  /// initial values of the lognormal uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Lognormal)
  RealVector lognormalUncVars;
  /// distribution lower bounds for the uniform uncertain variables
  /// (from the \c lower_bounds specification in \ref VarCAUV_Uniform)
  RealVector uniformUncLowerBnds;
  /// distribution upper bounds for the uniform uncertain variables
  /// (from the \c upper_bounds specification in \ref VarCAUV_Uniform)
  RealVector uniformUncUpperBnds;
  /// initial values of the uniform uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Uniform)
  RealVector uniformUncVars;
  /// distribution lower bounds for the loguniform uncertain variables
  /// (from the \c lower_bounds specification in \ref VarCAUV_Loguniform)
  RealVector loguniformUncLowerBnds;
  /// distribution upper bounds for the loguniform uncertain variables
  /// (from the \c upper_bounds specification in \ref VarCAUV_Loguniform)
  RealVector loguniformUncUpperBnds;
  /// initial values of the loguniform uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Loguniform)
  RealVector loguniformUncVars;
  /// modes of the triangular uncertain variables (from the \c modes
  /// specification in \ref VarCAUV_Triangular)
  RealVector triangularUncModes;
  /// distribution lower bounds for the triangular uncertain variables
  /// (from the \c lower_bounds specification in \ref VarCAUV_Triangular)
  RealVector triangularUncLowerBnds;
  /// distribution upper bounds for the triangular uncertain variables
  /// (from the \c upper_bounds specification in \ref VarCAUV_Triangular)
  RealVector triangularUncUpperBnds;
  /// initial values of the triangular uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Triangular)
  RealVector triangularUncVars;
  /// beta factors for the exponential uncertain variables (from
  /// the \c betas specification in \ref VarCAUV_Exponential)
  RealVector exponentialUncBetas;
  /// initial values of the exponential uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Exponential)
  RealVector exponentialUncVars;
  /// alpha factors for the beta uncertain variables (from
  /// the \c means specification in \ref VarCAUV_Beta)
  RealVector betaUncAlphas;
  /// beta factors for the beta uncertain variables (from
  /// the \c std_deviations specification in \ref VarCAUV_Beta)
  RealVector betaUncBetas;
  /// distribution lower bounds for the beta uncertain variables
  /// (from the \c lower_bounds specification in \ref VarCAUV_Beta)
  RealVector betaUncLowerBnds;
  /// distribution upper bounds for the beta uncertain variables
  /// (from the \c upper_bounds specification in \ref VarCAUV_Beta)
  RealVector betaUncUpperBnds;
  /// initial values of the beta uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Beta)
  RealVector betaUncVars;
  /// alpha factors for the gamma uncertain variables (from
  /// the \c alphas specification in \ref VarCAUV_Gamma)
  RealVector gammaUncAlphas;
  /// beta factors for the gamma uncertain variables (from
  /// the \c betas specification in \ref VarCAUV_Gamma)
  RealVector gammaUncBetas;
  /// initial values of the gamma uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Gamma)
  RealVector gammaUncVars;
  /// alpha factors for the gumbel uncertain variables (from the \c
  /// alphas specification in \ref VarCAUV_Gumbel)
  RealVector gumbelUncAlphas;
  /// beta factors for of the gumbel uncertain variables (from
  /// the \c betas specification in \ref VarCAUV_Gumbel)
  RealVector gumbelUncBetas;
  /// initial values of the gumbel uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Gumbel)
  RealVector gumbelUncVars;
  /// alpha factors for the frechet uncertain variables (from
  /// the \c alphas specification in \ref VarCAUV_Frechet)
  RealVector frechetUncAlphas;
  /// beta factors for the frechet uncertain variables (from
  /// the \c betas specification in \ref VarCAUV_Frechet)
  RealVector frechetUncBetas;
  /// initial values of the frechet uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Frechet)
  RealVector frechetUncVars;
  /// alpha factors for the weibull uncertain variables (from
  /// the \c alphas specification in \ref VarCAUV_Weibull)
  RealVector weibullUncAlphas;
  /// beta factors for the weibull uncertain variables (from
  /// the \c betas specification in \ref VarCAUV_Weibull)
  RealVector weibullUncBetas;
  /// initial values of the weibull uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Weibull)
  RealVector weibullUncVars;

  /// An array for each real-valued bin-based histogram uncertain
  /// variable. Each array entry is a map from a real value to its
  /// probability. (See continuous linear histogram in LHS manual;
  /// from the \c histogram_bin_uncertain specification in \ref
  /// VarCAUV_Bin_Histogram).  (x,y) ordinate specifications are
  /// converted to (x,c) counts within NIDR.
  RealRealMapArray histogramUncBinPairs;
  /// initial values of the histogram bin uncertain variables (from the \c
  /// initial_point specification in \ref VarCAUV_Bin_Histogram)
  RealVector histogramBinUncVars;

  // discrete types

  /// lambdas (rate parameter) for the poisson uncertain variables (from
  /// the \c lambdas specification in \ref VarDAUV_Poisson)
  RealVector poissonUncLambdas;
  /// initial values of the poisson uncertain variables (from the \c
  /// initial_point specification in \ref VarDAUV_Poisson)
  IntVector poissonUncVars;
  /// is each poisson var strictly categorical (true) or relaxable (false)
  BitArray poissonUncCat;

  /// probabilities per each trial (p) for the binomial uncertain variables
  /// from the \c prob_per_trial specification in \ref VarDAUV_Binomial)
  RealVector binomialUncProbPerTrial;
  /// Number of trials (N) for the binomial uncertain variables
  /// from the \c num_trials specification in \ref VarDAUV_Binomial)
  IntVector binomialUncNumTrials;
  /// initial values of the binomial uncertain variables (from the \c
  /// initial_point specification in \ref VarDAUV_Binomial)
  IntVector binomialUncVars;
  /// is each binomial var strictly categorical (true) or relaxable (false)
  BitArray binomialUncCat;

  /// probabilities per each trial (p) for the negative binomial
  /// uncertain variables from the \c prob_per_trial specification in
  /// \ref VarDAUV_Negative_Binomial)
  RealVector negBinomialUncProbPerTrial;
  /// Number of trials (N) for the negative binomial uncertain variables
  /// from the \c num_trials specification in \ref VarDAUV_Negative_Binomial)
  IntVector negBinomialUncNumTrials;
  /// initial values of the negative binomial uncertain variables (from the
  /// \c initial_point specification in \ref VarDAUV_Negative_Binomial)
  IntVector negBinomialUncVars;
  /// is each negbinomial var strictly categorical (true) or relaxable (false)
  BitArray negBinomialUncCat;

  /// probabilities per each trial (p) for the geometric uncertain variables
  /// from the \c prob_per_trial specification in \ref VarDAUV_Geometric)
  RealVector geometricUncProbPerTrial;
  /// initial values of the geometric uncertain variables (from the \c
  /// initial_point specification in \ref VarDAUV_Geometric)
  IntVector geometricUncVars;
  /// is each geometric var strictly categorical (true) or relaxable (false)
  BitArray geometricUncCat;

  /// Size of total populations (N) for the hypergeometric uncertain variables
  /// from the \c total_population specification in \ref VarDAUV_Hypergeometric)
  IntVector hyperGeomUncTotalPop;
  /// Size of selected populations for the hypergeometric uncertain
  /// variables from the \c selected_population specification in \ref
  /// VarDAUV_Hypergeometric)
  IntVector hyperGeomUncSelectedPop;
  /// Number failed in the selected populations for the hypergeometric variables
  /// from the \c num_drawn specification in \ref VarDAUV_Hypergeometric)
  IntVector hyperGeomUncNumDrawn;
  /// initial values of the hypergeometric uncertain variables (from the
  /// \c initial_point specification in \ref VarDAUV_Hypergeometric)
  IntVector hyperGeomUncVars;
  /// is each hypergeom var strictly categorical (true) or relaxable (false)
  BitArray hyperGeomUncCat;

  /// An array for each integer-valued point-based histogram uncertain
  /// variable. Each array entry is a map from an integer value to its
  /// probability. (See discrete histogram in LHS manual; from the \c
  /// histogram_point_uncertain specification in \ref
  /// VarDAUV_Point_Histogram)
  IntRealMapArray histogramUncPointIntPairs;
  /// initial values of the real-valued histogram point uncertain
  /// variables (from the \c initial_point specification in \ref
  /// VarDAUV_Point_Histogram)
  IntVector histogramPointIntUncVars;
  /// is each hupi var strictly categorical (true) or relaxable (false)
  BitArray histogramUncPointIntCat;
 
  /// An array for each string-valued point-based histogram uncertain
  /// variable. Each array entry is a map from a string value to its
  /// probability. (See discrete histogram in LHS manual; from the \c
  /// histogram_point_uncertain specification in \ref
  /// VarDAUV_Point_Histogram)
  StringRealMapArray histogramUncPointStrPairs;
  /// initial values of the real-valued histogram point uncertain
  /// variables (from the \c initial_point specification in \ref
  /// VarDAUV_Point_Histogram)
  StringArray histogramPointStrUncVars;
  // string variables cannot be relaxed, no categorical option

  /// An array for each real-valued point-based histogram uncertain
  /// variable. Each array entry is a map from a real value to its
  /// probability. (See discrete histogram in LHS manual; from the \c
  /// histogram_point_uncertain specification in \ref
  /// VarDAUV_Point_Histogram)
  RealRealMapArray histogramUncPointRealPairs;
  /// initial values of the real-valued histogram point uncertain
  /// variables (from the \c initial_point specification in \ref
  /// VarDAUV_Point_Histogram)
  RealVector histogramPointRealUncVars;
  /// is each hupr var strictly categorical (true) or relaxable (false)
  BitArray histogramUncPointRealCat;

  /// correlation matrix for all uncertain variables (from the \c
  /// uncertain_correlation_matrix specification in \ref
  /// VarAUV_Correlations). This matrix specifies rank correlations
  /// for LHS sampling and correlation coefficients (rho_ij =
  /// normalized covariance matrix) for other methods.
  RealSymMatrix uncertainCorrelations;

  // Epistemic uncertain arrays

  /// Probability values per interval cell per epistemic interval uncertain
  /// variable (from the \c continuous_interval_uncertain \c interval_probs
  /// specification in \ref VarCEUV_Interval)
  RealRealPairRealMapArray continuousIntervalUncBasicProbs;
  /// initial values of the continuous interval uncertain variables
  /// (from the \c initial_point specification in \ref VarCEUV_Interval)
  RealVector continuousIntervalUncVars;

  /// Probability values per interval cell per epistemic interval uncertain
  /// variable (from the \c discrete_interval_uncertain \c interval_probs
  /// specification in \ref VarDIUV)
  IntIntPairRealMapArray discreteIntervalUncBasicProbs;
  /// initial values of the discrete interval uncertain variables
  /// (from the \c initial_point specification in \ref VarDIUV)
  IntVector discreteIntervalUncVars;
  /// is each diu var strictly categorical (true) or relaxable (false)
  BitArray discreteIntervalUncCat;

  /// complete set of admissible values with associated basic probability
  /// assignments for each of the discrete epistemic uncertain variables
  /// defined by an integer set (from the \c discrete_uncertain_set integer
  /// \c set_values specification in \ref VarDUSIV)
  IntRealMapArray discreteUncSetIntValuesProbs;
  /// initial values of the discrete uncertain set integer variables
  /// (from the \c initial_point specification in \ref VarDUSIV)
  IntVector discreteUncSetIntVars;
  /// is each dusi var strictly categorical (true) or relaxable (false)
  BitArray discreteUncSetIntCat;

  /// complete set of admissible values with associated basic probability
  /// assignments for each of the discrete epistemic uncertain variables
  /// defined by a string set (from the \c discrete_uncertain_set string
  /// \c set_values specification in \ref VarDUSIV)
  StringRealMapArray discreteUncSetStrValuesProbs;
  /// initial values of the discrete uncertain set integer variables
  /// (from the \c initial_point specification in \ref VarDUSIV)
  StringArray discreteUncSetStrVars;

  /// complete set of admissible values with associated basic probability
  /// assignments for each of the discrete epistemic uncertain variables
  /// defined by a real set (from the \c discrete_uncertain_set_ eal
  /// \c set_values specification in \ref VarDUSRV)
  RealRealMapArray discreteUncSetRealValuesProbs;
  /// initial values of the discrete uncertain set real variables
  /// (from the \c initial_point specification in \ref VarDUSRV)
  RealVector discreteUncSetRealVars;
  /// is each dusr var strictly categorical (true) or relaxable (false)
  BitArray discreteUncSetRealCat;

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
  /// is each dsr var strictly categorical (true) or relaxable (false)
  BitArray discreteStateRangeCat;

  /// initial values for the discrete state variables defined by an
  /// integer set (from the \c discrete_state_set integer \c initial_point
  /// specification in \ref VarSV)
  IntVector discreteStateSetIntVars;
  /// initial values for the discrete state variables defined by a
  /// string set (from the \c discrete_state_set string \c initial_point
  /// specification in \ref VarSV)
  StringArray discreteStateSetStrVars;
  /// initial values for the discrete state variables defined by a
  /// real set (from the \c discrete_state_set real \c initial_point
  /// specification in \ref VarSV)
  RealVector discreteStateSetRealVars;

  /// complete set of admissible values for each of the discrete state
  /// variables defined by an integer set (from the
  /// \c discrete_state_set integer \c set_values specification in \ref VarSV)
  IntSetArray discreteStateSetInt;
  /// complete set of admissible values for each of the discrete state
  /// variables defined by a string set (from the \c discrete_state_set string
  /// \c set_values specification in \ref VarSV)
  StringSetArray discreteStateSetStr;
  /// complete set of admissible values for each of the discrete state
  /// variables defined by a real set (from the \c discrete_state_set real
  /// \c set_values specification in \ref VarSV)
  RealSetArray discreteStateSetReal;

  /// is each dssi var strictly categorical (true) or relaxable (false)
  BitArray discreteStateSetIntCat;
  /// is each dssr var strictly categorical (true) or relaxable (false)
  BitArray discreteStateSetRealCat;

  /// labels array for the continuous state variables (from the
  /// \c continuous_state \c descriptors specification in \ref VarSV)
  StringArray continuousStateLabels;
  /// labels array for the discrete state variables defined by an
  /// integer range (from the \c discrete_state_range \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateRangeLabels;
  /// labels array for the discrete state variables defined by an
  /// integer set (from the \c discrete_state_set \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateSetIntLabels;
  /// labels array for the discrete state variables defined by a
  /// string set (from the \c discrete_state_set \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateSetStrLabels;
  /// labels array for the discrete state variables defined by a
  /// real set (from the \c discrete_state_set \c descriptors
  /// specification in \ref VarSV)
  StringArray discreteStateSetRealLabels;

  // Inferred arrays

  /// discrete design integer set lower bounds inferred from set values
  IntVector discreteDesignSetIntLowerBnds;
  /// discrete design integer set upper bounds inferred from set values
  IntVector discreteDesignSetIntUpperBnds;
  /// discrete design string set lower bounds inferred from set values
  StringArray discreteDesignSetStrLowerBnds;
  /// discrete design stinr set upper bounds inferred from set values
  StringArray discreteDesignSetStrUpperBnds;
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

  /// array of values for all discrete string epistemic uncertain variables
  StringArray discreteStrAleatoryUncVars;
  /// distribution lower bounds for all discrete string epistemic
  /// uncertain variables
  StringArray discreteStrAleatoryUncLowerBnds;
  /// distribution upper bounds for all discrete string epistemic
  /// uncertain variables
  StringArray discreteStrAleatoryUncUpperBnds;
  /// labels for all discrete string epistemic uncertain variables
  StringArray discreteStrAleatoryUncLabels;

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

  /// array of values for all discrete string epistemic uncertain variables
  StringArray discreteStrEpistemicUncVars;
  /// distribution lower bounds for all discrete string epistemic
  /// uncertain variables
  StringArray discreteStrEpistemicUncLowerBnds;
  /// distribution upper bounds for all discrete string epistemic
  /// uncertain variables
  StringArray discreteStrEpistemicUncUpperBnds;
  /// labels for all discrete string epistemic uncertain variables
  StringArray discreteStrEpistemicUncLabels;

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
  /// discrete state string set lower bounds inferred from set values
  StringArray discreteStateSetStrLowerBnds;
  /// discrete state string set upper bounds inferred from set values
  StringArray discreteStateSetStrUpperBnds;
  /// discrete state real set lower bounds inferred from set values
  RealVector discreteStateSetRealLowerBnds;
  /// discrete state real set upper bounds inferred from set values
  RealVector discreteStateSetRealUpperBnds;

  // Linear constraints

  /// coefficient matrix for the linear inequality constraints (from
  /// the \c linear_inequality_constraint_matrix specification in \ref
  /// MethodIndControl)
  RealVector linearIneqConstraintCoeffs;
  /// lower bounds for the linear inequality constraints (from the \c
  /// linear_inequality_lower_bounds specification in \ref MethodIndControl)
  RealVector linearIneqLowerBnds;
  /// upper bounds for the linear inequality constraints (from the \c
  /// linear_inequality_upper_bounds specification in \ref MethodIndControl)
  RealVector linearIneqUpperBnds;
  /// scaling types for the linear inequality constraints (from the \c
  /// linear_inequality_scale_types specification in \ref MethodIndControl)
  StringArray linearIneqScaleTypes;
  /// scaling factors for the linear inequality constraints (from the \c
  /// linear_inequality_scales specification in \ref MethodIndControl)
  RealVector linearIneqScales;
  /// coefficient matrix for the linear equality constraints (from
  /// the \c linear_equality_constraint_matrix specification in \ref
  /// MethodIndControl)
  RealVector linearEqConstraintCoeffs;
  /// targets for the linear equality constraints (from the \c
  /// linear_equality_targets specification in \ref MethodIndControl)
  RealVector linearEqTargets;
  /// scaling types for the linear equality constraints (from the \c
  /// linear_equality_scale_types specification in \ref MethodIndControl)
  StringArray linearEqScaleTypes;
  /// scaling factors for the linear equality constraints (from the \c
  /// linear_equality_scales specification in \ref MethodIndControl)
  RealVector linearEqScales;

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

  /// return dataVarsRep
  DataVariablesRep* data_rep();

  // Get Functions (composite variable counts only):

  /// return total number of design variables
  size_t design() { return dataVarsRep->numContinuousDesVars +
    dataVarsRep->numDiscreteDesRangeVars +
    dataVarsRep->numDiscreteDesSetIntVars +
    dataVarsRep->numDiscreteDesSetStrVars +
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
    dataVarsRep->numHistogramPtIntUncVars + 
    dataVarsRep->numHistogramPtStrUncVars + 
    dataVarsRep->numHistogramPtRealUncVars; }
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
    dataVarsRep->numDiscreteStateSetStrVars +
    dataVarsRep->numDiscreteStateSetRealVars; }
  /// return total number of continuous variables
  size_t continuous_variables() { return dataVarsRep->numContinuousDesVars +
    uncertain() + dataVarsRep->numContinuousStateVars; }
  /// return total number of discrete variables
  // BMA TODO: should point histogram be included here?
  size_t discrete_variables()   { return dataVarsRep->numDiscreteDesRangeVars +
    dataVarsRep->numDiscreteDesSetIntVars +
    dataVarsRep->numDiscreteDesSetStrVars +
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


inline DataVariablesRep* DataVariables::data_rep()
{return dataVarsRep; }


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
