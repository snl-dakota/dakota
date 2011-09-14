/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataMethod
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataMethod.H 6984 2010-09-27 02:11:09Z lpswile $

#ifndef DATA_METHOD_H
#define DATA_METHOD_H

#include "system_defs.h"
#include "data_types.h"
#include "MPIPackBuffer.H"
#include "DakotaString.H"

namespace Dakota {

// --------
// Iterator
// --------
// define special values for outputLevel within
// Iterator/Model/Interface/Approximation/AnalysisCode
enum { SILENT_OUTPUT, QUIET_OUTPUT, NORMAL_OUTPUT, VERBOSE_OUTPUT,
       DEBUG_OUTPUT };

// ----
// NonD
// ----
// define special values for u_space_type in
// NonD::initialize_random_variable_types()
enum { STD_NORMAL_U, ASKEY_U, EXTENDED_U, PIECEWISE_U };
// define special values for probability integration refinement
enum { NO_INT_REFINE, IS, AIS, MMAIS };
// define special values for respLevelTarget
enum { PROBABILITIES, RELIABILITIES, GEN_RELIABILITIES };

// -------------
// NonDExpansion (enums delegated to Pecos in pecos_global_defs.hpp)
// -------------

// --------------------
// NonDBayesCalibration
// --------------------
// define special values for emulatorType
enum { NO_EMULATOR, POLYNOMIAL_CHAOS, STOCHASTIC_COLLOCATION,
       GAUSSIAN_PROCESS };

// ------------
// NonDSampling
// ------------
// LHS rank array processing modes:
enum { IGNORE_RANKS, SET_RANKS, GET_RANKS, SET_GET_RANKS };
// sampling modes (combinations of Uncertain/Active/All and Native/Uniform):
enum { UNCERTAIN, UNCERTAIN_UNIFORM, ACTIVE, ACTIVE_UNIFORM, ALL, ALL_UNIFORM };
// (1) UNCERTAIN: sample only over the uncertain variables, ignoring
//     design/state, using the native distributions.
// (2) UNCERTAIN_UNIFORM: sample only over the uncertain variables, ignoring
//     design/state, using uniform distributions.
// (3) ACTIVE: sample only over the active variables, ignoring inactive, using
//     the native distributions (assumes uniform for design/state).
// (4) ACTIVE_UNIFORM: sample only over the active variables, ignoring inactive,
//     using uniform distributions.
// (5) ALL: sample over All variables using native distributions (assumes
//     uniform for design/state).
// (6) ALL_UNIFORM: sample over All variables using uniform distributions.
// Note that ACTIVE modes will overlap with either UNCERTAIN or ALL modes
// depending on whether the Model/Variables employ an All or Distinct view.

// --------------------
// NonDLocalReliability
// --------------------
// define special values for mppSearchType
enum { MV, AMV_X, AMV_U, AMV_PLUS_X, AMV_PLUS_U, TANA_X, TANA_U, NO_APPROX };
// define special values for secondOrderIntType
enum { BREITUNG, HOHENRACK, HONG };

// ---------------------
// NonDGlobalReliability
// ---------------------
// define special values for mppSearchType
enum { EGRA_X, EGRA_U };

// -----------------------
// SurrBasedLocalMinimizer
// -----------------------
// define special values for SBL approximate subproblem objective
enum { ORIGINAL_PRIMARY, SINGLE_OBJECTIVE, 
       LAGRANGIAN_OBJECTIVE, AUGMENTED_LAGRANGIAN_OBJECTIVE };
// define special values for SBL approximate subproblem constraints
enum { NO_CONSTRAINTS, LINEARIZED_CONSTRAINTS, ORIGINAL_CONSTRAINTS };
// define special values for SBL constraint relaxation
enum { NO_RELAX, HOMOTOPY, COMPOSITE_STEP }; // COMPOSITE_STEP: BYRD_OMOJOKUN,
                                             // CELIS_DENNIS_TAPIA, or MAESTRO
// define special values for SBL and NonDGlobalReliability merit function type
enum { PENALTY_MERIT,    ADAPTIVE_PENALTY_MERIT,
       LAGRANGIAN_MERIT, AUGMENTED_LAGRANGIAN_MERIT }; // FILTER_AREA
// define special values for SBL iterate acceptance logic
enum { FILTER, TR_RATIO };

// ---------
// Minimizer
// ---------

// minimum value allowed for a characteristic value when scaling; ten
// orders of magnitude greater than DBL_MIN
#define SCALING_MIN_SCALE 1.0e10*DBL_MIN
// lower bound on domain of logarithm function when scaling
#define SCALING_MIN_LOG SCALING_MIN_SCALE
// logarithm base to be used when scaling
#define SCALING_LOGBASE 10.0
// ln(SCALING_LOGBASE); needed in transforming variables in several places
#define SCALING_LN_LOGBASE log(SCALING_LOGBASE)
// indicate type of scaling active for a component (bitwise)
enum { SCALE_NONE, SCALE_VALUE, SCALE_LOG };
// to indicate type of object being scaled
enum { CDV, LINEAR, NONLIN, FN_LSQ };
// to restrict type of auto scaling allowed
enum { DISALLOW, TARGET, BOUNDS };


/// Body class for method specification data.

/** The DataMethodRep class is used to contain the data from a method
    keyword specification.  Default values are managed in the
    DataMethodRep constructor.  Data is public to avoid maintaining
    set/get functions, but is still encapsulated within ProblemDescDB
    since ProblemDescDB::dataMethodList is private. */

class DataMethodRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataMethod;

public:

  //
  //- Heading: Data
  //

  // method independent controls

  /// string identifier for the method specification data set (from
  /// the \c id_method specification in \ref MethodIndControl)
  String idMethod;
  /// string pointer to the model specification to be used by this method
  /// (from the \c model_pointer specification in \ref MethodIndControl)
  String modelPointer;
  /// method verbosity control: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  /// (from the \c output specification in \ref MethodIndControl)
  short methodOutput;
  /// maximum number of iterations allowed for the method (from the \c
  /// max_iterations specification in \ref MethodIndControl)
  int maxIterations;
  /// maximum number of function evaluations allowed for the method (from
  /// the \c max_function_evaluations specification in \ref MethodIndControl)
  int maxFunctionEvaluations;
  /// flag for use of speculative gradient approaches for maintaining parallel
  /// load balance during the line search portion of optimization algorithms
  /// (from the \c speculative specification in \ref MethodIndControl)
  bool speculativeFlag;
  /// flag for usage of derivative data to enhance the computation of
  /// surrogate models (PCE/SC expansions, GP models for EGO/EGRA/EGIE)
  /// based on the \c use_derivatives specification
  bool methodUseDerivsFlag;
  /// iteration convergence tolerance for the method (from the \c
  /// convergence_tolerance specification in \ref MethodIndControl)
  Real convergenceTolerance;
  /// tolerance for controlling the amount of infeasibility that is allowed
  /// before an active constraint is considered to be violated (from the \c
  /// constraint_tolerance specification in \ref MethodIndControl)
  Real constraintTolerance;
  /// flag indicating scaling status (from the \c scaling specification in 
  /// \ref MethodIndControl)
  bool methodScaling;
  /// number of final solutions returned from the iterator
  size_t numFinalSolutions;

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

  /// the method selection: one of the optimizer, least squares, nond, dace,
  /// or parameter study methods
  String methodName;
  /// string identifier for a sub-method within a multi-option method
  /// specification (e.g., from \c sub_method_name in SBL/SBG, \c dace
  /// option, or \c richardson_extrap option)
  String subMethodName;
  /// string pointer for a sub-method specification used by a multi-component
  /// method (from the \c sub_method_pointer specification in SBL/SBG)
  String subMethodPointer;

  /// number of consecutive iterations with change less than
  /// convergenceTolerance required to trigger convergence within
  /// the surrogate-based local method (from the \c
  /// soft_convergence_limit specification in \ref MethodSBL)
  int surrBasedLocalSoftConvLimit;
  /// flag to indicate user-specification of a bypass of any/all
  /// layerings in evaluating truth response values in SBL.
  bool surrBasedLocalLayerBypass;
  /// initial trust region size in the surrogate-based local method
  /// (from the \c initial_size specification in \ref MethodSBL) note:
  /// this is a relative value, e.g., 0.1 = 10% of global bounds
  /// distance (upper bound - lower bound) for each variable
  Real surrBasedLocalTRInitSize;
  /// minimum trust region size in the surrogate-based local method
  /// (from the \c minimum_size specification in \ref MethodSBL), if
  /// the trust region size falls below this threshold the SBL
  /// iterations are terminated (note: if kriging is used with SBL,
  /// the min trust region size is set to 1.0e-3 in attempt to avoid
  /// ill-conditioned matrixes that arise in kriging over small trust
  /// regions)
  Real surrBasedLocalTRMinSize;
  /// trust region minimum improvement level (ratio of actual to predicted
  /// decrease in objective fcn) in the surrogate-based local method
  /// (from the \c contract_threshold specification in \ref MethodSBL),
  /// the trust region shrinks or is rejected if the ratio is below
  /// this value ("eta_1" in the Conn-Gould-Toint trust region book)
  Real surrBasedLocalTRContractTrigger;
  /// trust region sufficient improvement level (ratio of actual to
  /// predicted decrease in objective fn) in the surrogate-based local
  /// method (from the \c expand_threshold specification in \ref
  /// MethodSBL), the trust region expands if the ratio is above this
  /// value ("eta_2" in the Conn-Gould-Toint trust region book)
  Real surrBasedLocalTRExpandTrigger;
  /// trust region contraction factor in the surrogate-based local method
  /// (from the \c contraction_factor specification in \ref MethodSBL)
  Real surrBasedLocalTRContract;
  /// trust region expansion factor in the surrogate-based local method
  /// (from the \c expansion_factor specification in \ref MethodSBL)
  Real surrBasedLocalTRExpand;
  /// SBL approximate subproblem objective: ORIGINAL_PRIMARY, SINGLE_OBJECTIVE,
  /// LAGRANGIAN_OBJECTIVE, or AUGMENTED_LAGRANGIAN_OBJECTIVE
  short surrBasedLocalSubProbObj;
  /// SBL approximate subproblem constraints: NO_CONSTRAINTS,
  /// LINEARIZED_CONSTRAINTS, or ORIGINAL_CONSTRAINTS
  short surrBasedLocalSubProbCon;
  /// SBL merit function type: BASIC_PENALTY, ADAPTIVE_PENALTY, 
  /// BASIC_LAGRANGIAN, or AUGMENTED_LAGRANGIAN
  short surrBasedLocalMeritFn;
  /// SBL iterate acceptance logic: TR_RATIO or FILTER
  short surrBasedLocalAcceptLogic;
  /// SBL constraint relaxation method: NO_RELAX or HOMOTOPY
  short surrBasedLocalConstrRelax;
  /// user-specified method for adding points to the set upon which the
  /// next surrogate is based in the \c surrogate_based_global strategy. 
  bool surrBasedGlobalReplacePts;

  // number of samples at the root for the branch and bound method
  // (from the \c num_samples_at_root specification in \ref MethodBandB)
  //int branchBndNumSamplesRoot;
  // number of samples at each node for the branch and bound method
  // (from the \c num_samples_at_node specification in \ref MethodBandB)
  //int branchBndNumSamplesNode;

  // DOT and CONMIN
  /// the \c optimization_type specification in \ref MethodDOTDC
  String minMaxType;

  // DL_SOLVER

  /// string of options for a dynamically linked solver
  String dlDetails;
  /// handle to dynamically loaded library
  void *dlLib;

  // NPSOL

  /// the \c verify_level specification in \ref MethodNPSOLDC
  int verifyLevel;
  /// the \c function_precision specification in \ref MethodNPSOLDC
  Real functionPrecision;	/* also used by nl2sol */
  /// the \c linesearch_tolerance specification in \ref MethodNPSOLDC
  Real lineSearchTolerance;

  // NL2SOL

  Real absConvTol;     ///< absolute function convergence tolerance
  Real xConvTol;       ///< x-convergence tolerance
  Real singConvTol;    ///< singular convergence tolerance
  Real singRadius;     ///< radius for singular convergence test
  Real falseConvTol;   ///< false-convergence tolerance
  Real initTRRadius;   ///< initial trust radius
  int  covarianceType; ///< kind of covariance required
  bool regressDiag;    ///< whether to print the regression diagnostic vector

  // OPT++

  /// the \c search_method specification for Newton and nonlinear
  /// interior-point methods in \ref MethodOPTPPDC
  String searchMethod;
  /// the \c gradient_tolerance specification in \ref MethodOPTPPDC
  Real gradientTolerance;
  /// the \c max_step specification in \ref MethodOPTPPDC
  Real maxStep;
  /// the \c merit_function specification for nonlinear
  /// interior-point methods in \ref MethodOPTPPDC
  String meritFn;
  /// the \c central_path specification for nonlinear interior-point
  /// methods in \ref MethodOPTPPDC
  String centralPath;
  /// the \c steplength_to_boundary specification for nonlinear
  /// interior-point methods in \ref MethodOPTPPDC
  Real stepLenToBoundary;
  /// the \c centering_parameter specification for nonlinear
  /// interior-point methods in \ref MethodOPTPPDC
  Real centeringParam;
  /// the \c search_scheme_size specification for PDS methods in
  /// \ref MethodOPTPPDC
  int  searchSchemeSize;

  // APPSPACK

  // using solnTarget from COLINY

  /// the \c initStepLength choice for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  Real initStepLength;
  /// the \c contractStepLength choice for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  Real contractStepLength;
  /// the \c threshStepLength choice for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  Real threshStepLength;
  /// the \c synchronization choice for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  String evalSynchronize;
  /// the \c meritFunction choice for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  String meritFunction;
  /// the \c constrPenalty choice for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  Real constrPenalty;
  /// the initial \c smoothFactor value for nonlinearly constrained APPS in
  /// \ref MethodAPPSDC
  Real smoothFactor;

  // COLINY

  /// the \c synchronization setting for parallel pattern search
  /// methods in \ref MethodCOLINYPS and \ref MethodAPPS
  String evalSynchronization;
  /// the initial \c constraint_penalty for COLINY methods in
  /// \ref MethodAPPS, \ref MethodCOLINYDIR, \ref MethodCOLINYPS,
  /// \ref MethodCOLINYSW and \ref MethodCOLINYEA
  Real constraintPenalty;
  /// the \c constant_penalty flag for COLINY methods in
  /// \ref MethodCOLINYPS and \ref MethodCOLINYSW
  bool constantPenalty;
  /// the \c global_balance_parameter for the DIRECT method in
  /// \ref MethodCOLINYDIR
  Real globalBalanceParam;
  /// the \c local_balance_parameter for the DIRECT method in
  /// \ref MethodCOLINYDIR
  Real localBalanceParam;
  /// the \c max_boxsize_limit for the DIRECT method in \ref MethodCOLINYDIR
  Real maxBoxSize;
  /// the \c min_boxsize_limit for the DIRECT method in \ref MethodCOLINYDIR
  /// and \ref MethodNCSUDC
  Real minBoxSize;
  /// the \c division setting (\c major_dimension or \c all_dimensions) for
  /// the DIRECT method in \ref MethodCOLINYDIR
  String boxDivision;
  /// the \c non_adaptive specification for the coliny_ea method in
  /// \ref MethodCOLINYEA
  bool mutationAdaptive;
  /// the \c show_misc_options specification in \ref MethodCOLINYDC
  bool showMiscOptions;
  /// the \c misc_options specification in \ref MethodCOLINYDC
  StringArray miscOptions;
  /// the \c solution_target specification in \ref MethodCOLINYDC
  Real solnTarget;
  /// the \c crossover_rate specification for EA methods in \ref MethodCOLINYEA
  Real crossoverRate;
  /// the \c mutation_rate specification for EA methods in \ref  MethodCOLINYEA
  Real mutationRate;
  /// the \c mutation_scale specification for EA methods in \ref  MethodCOLINYEA
  Real mutationScale;
  /// the \c min_scale specification for mutation in EA methods in
  /// \ref MethodCOLINYEA
  Real mutationMinScale;
  /// the \c initial_delta specification for APPS/COBYLA/PS/SW methods in 
  /// \ref MethodAPPS, \ref MethodCOLINYCOB, \ref MethodCOLINYPS, and
  /// \ref MethodCOLINYSW
  Real initDelta;
  /// the \c threshold_delta specification for APPS/COBYLA/PS/SW methods
  /// in \ref MethodAPPS, \ref MethodCOLINYCOB, \ref MethodCOLINYPS, and
  /// \ref MethodCOLINYSW
  Real threshDelta;
  /// the \c contraction_factor specification for APPS/PS/SW methods in
  /// \ref MethodAPPS, \ref MethodCOLINYPS, and \ref MethodCOLINYSW
  Real contractFactor;
  /// the \c new_solutions_generated specification for GA/EPSA methods
  /// in \ref MethodCOLINYEA
  int newSolnsGenerated;
  /// the integer assignment to random, chc, or elitist in the \c
  /// replacement_type specification for GA/EPSA methods in \ref
  /// MethodCOLINYEA
  int numberRetained;
  /// the \c no_expansion specification for APPS/PS/SW methods in
  /// \ref MethodAPPS, \ref MethodCOLINYPS, and \ref MethodCOLINYSW
  bool expansionFlag;
  /// the \c expand_after_success specification for PS/SW methods in
  /// \ref MethodCOLINYPS and \ref MethodCOLINYSW
  int expandAfterSuccess;
  /// the \c contract_after_failure specification for the SW method in
  /// \ref MethodCOLINYSW
  int contractAfterFail;
  /// the \c mutation_range specification for the pga_int method in
  /// \ref MethodCOLINYEA
  int mutationRange;
  /// the \c total_pattern_size specification for PS methods in
  /// \ref MethodCOLINYPS
  int totalPatternSize;
  /// the \c stochastic specification for the PS method in \ref MethodCOLINYPS
  bool randomizeOrderFlag;
  /// the \c fitness_type specification for EA methods in \ref MethodCOLINYEA
  String selectionPressure;
  /// the \c replacement_type specification for EA methods in
  /// \ref MethodCOLINYEA
  String replacementType;
  /// the \c crossover_type specification for EA methods in \ref MethodCOLINYEA
  String crossoverType;
  /// the \c mutation_type specification for EA methods in \ref MethodCOLINYEA
  String mutationType;
  /// the \c exploratory_moves specification for the PS method in
  /// \ref MethodCOLINYPS
  String exploratoryMoves;
  /// the \c pattern_basis specification for APPS/PS methods in
  /// \ref MethodAPPS and \ref MethodCOLINYPS
  String patternBasis;

  // JEGA

  // using randomSeed from COLINY, NonD, & DACE methods
  // using mutationType from COLINY
  // using crossoverType from COLINY
  // using mutationRate from COLINY
  // using mutationScale from COLINY
  // using crossoverRate from COLINY
  // using populationSize from COLINY
  // using maxIterations from method independent controls
  // using maxFunctionEvaluations from method independent controls
  // using convergenceTolerance for percentChange from method ind. controls
  // mainLoopType defined in JEGAOptimizer.C
  // evaluationType defined in JEGAOptimizer.C
  // delimiter for flat file read defined in JEGAOptimizer.C

  // varibles for the crossover operator
  /// The number of crossover points or multi-point schemes.
  size_t numCrossPoints;
  /// The number of parents to use in a crossover operation.
  size_t numParents;
  /// The number of children to produce in a crossover operation.
  size_t numOffspring;
  
  // variables for the fitness assessment operator
  /// the fitness assessment operator to use.
  String fitnessType;
  
  // variables for the selection operator
  /// The means by which this JEGA should converge.
  String convergenceType;
  
  /// The minimum percent change before convergence
  /// for a fitness tracker converger.
  Real percentChange;
  /// The number of generations over which a fitness
  /// tracker converger should track.
  size_t numGenerations;

  // JEMOGA
  /// The cutoff value for survival in fitness limiting selectors (e.g., 
  /// below_limit selector).
  Real fitnessLimit;
  /// The minimum percentage of the requested number of selections that
  /// must take place on each call to the selector (0, 1).
  Real shrinkagePercent;
   
  // variables for the niching operator
  /// The niching type 
  String nichingType;

  // variables for the niching type
  /// The discretization percentage along each objective
  RealVector nicheVector;
  
  // variables for the postprocessor operator
  /// The post processor type 
  String postProcessorType;

  // variables for the postprocessor type
  /// The discretization percentage along each objective
  RealVector distanceVector;
  
  // JESOGA


  // JEGA/COLINY

  // variables for initialization
  /// The means by which the JEGA should initialize the population.
  String initializationType;
  /// The filename to use for initialization.
  String flatFile;
  /// The filename to use for logging
  String logFile;
  /// the \c population_size specification for GA methods in \ref
  /// MethodCOLINYEA
  int populationSize;
  /// The \c print_each_pop flag to set the printing of the population 
  /// at each generation
  bool printPopFlag;

  // NCSU

  // using solnTarget from COLINY
  // using minBoxSize from COLINY

  /// the \c volume_boxsize_limit for the DIRECT method in \ref MethodNCSUDC
  Real volBoxSize;

  // DDACE

  /// the \c symbols specification for DACE methods
  int numSymbols;
  /// the \c main_effects specification for sampling methods 
  /// in \ref MethodDDACE)
  bool mainEffectsFlag;

  // FSUDace

  // using numSamples from DDACE

  /// the \c latinize specification for FSU QMC and CVT methods in
  /// \ref MethodFSUDACE
  bool latinizeFlag;
  /// the \c quality_metrics specification for sampling methods (FSU QMC 
  /// and CVT methods in \ref MethodFSUDACE)
  bool volQualityFlag;

  // FSUDace QMC
  /// the \c sequenceStart specification in \ref MethodFSUDACE
  IntVector sequenceStart;
  /// the \c sequenceLeap specification in \ref MethodFSUDACE
  IntVector sequenceLeap;
  /// the \c primeBase specification in \ref MethodFSUDACE
  IntVector primeBase;

  // FSUDace CVT
  // using randomSeed, fixedSeedFlag, numSamples from other methods 
  // using initializationType, sampleType from other methods
  /// the \c numTrials specification in \ref MethodFSUDACE
  int numTrials;
  /// the \c trial_type specification in \ref MethodFSUDACE 
  String trialType;
 
  // COLINY, NonD, & DACE

  /// the \c seed specification for COLINY, NonD, & DACE methods
  int randomSeed;

  // NonD & DACE

  /// the \c samples specification for NonD & DACE methods
  int numSamples;
  /// flag for fixing the value of the seed among different NonD/DACE
  /// sample sets.  This results in the use of the same sampling
  /// stencil/pattern throughout a strategy with repeated sampling.
  bool fixedSeedFlag;
  /// flag for fixing the sequence for Halton or Hammersley QMC
  /// sample sets.  This results in the use of the same sampling
  /// stencil/pattern throughout a strategy with repeated sampling.
  bool fixedSequenceFlag;
  /// the number of previous samples when augmenting a LHS sample
  int previousSamples;
  /// the \c var_based_decomp specification for a variety of sampling methods
  bool vbdFlag;
  /// the \c var_based_decomp tolerance for omitting index output
  Real vbdDropTolerance;

  // NonD

  /// a sub-specification of vbdFlag: {NO,UNIVARIATE,ALL}_VBD.  When
  /// vbdFlag is on, controls granularity of calculation/output of
  /// main/interaction/total effects
  short vbdControl;
  /// the \c basic random-number generator for NonD
  String rngName;
  /// refinement type for stochastic expansions from dimension refinement
  /// keyword group
  short refinementType;
  /// refinement control for stochastic expansions from dimension refinement
  /// keyword group
  short refinementControl;
  /// override for default point nesting policy: NO_NESTING_OVERRIDE, NESTED,
  /// or NON_NESTED
  short nestingOverride;
  /// override for default point growth restriction policy: NO_GROWTH_OVERRIDE,
  /// RESTRICTED, or UNRESTRICTED
  short growthOverride;
  /// enumeration for overriding the u-space type default from EXTENDED_U to
  /// ASKEY_U, STD_NORMAL_U, or PIECEWISE_U based on input keywords \c askey,
  /// \c wiener, or \c piecewise, respectively
  short expansionType;
  /// the \c expansion_terms specification in \ref MethodNonDPCE
  int expansionTerms;
  /// the \c expansion_order specification in \ref MethodNonDPCE
  UShortArray expansionOrder;
  /// the \c expansion_samples specification in \ref MethodNonDPCE
  int expansionSamples;
  /// allows for incremental PCE construction using the \c
  /// incremental_lhs specification in \ref MethodNonDPCE
  String expansionSampleType;
  /// the \c quadrature_order specification in \ref MethodNonDPCE and
  /// \ref MethodNonDSC
  UShortArray quadratureOrder;
  /// the \c sparse_grid_level specification in \ref MethodNonDPCE,
  /// \ref MethodNonDSC, and other stochastic expansion-enabled methods
  UShortArray sparseGridLevel;
  /// the \c dimension_preference specification for tensor and sparse grids
  /// in \ref MethodNonDPCE and \ref MethodNonDSC
  RealVector anisoGridDimPref;
  /// the \c cubature_integrand specification in \ref MethodNonDPCE
  unsigned short cubIntOrder;
  /// the \c collocation_points specification in \ref MethodNonDPCE
  int collocationPoints;
  /// the \c collocation_ratio specification in \ref MethodNonDPCE
  Real collocationRatio;
  /// flag indicating application of the collocation ratio to the
  /// square of the number of expansion terms within regression PCE;
  /// based on the \c squared specification in \ref MethodNonDPCE
  bool collocRatioSquaredTermsFlag;
  /// allows for incremental PCE construction using the \c
  /// reuse_points specification in \ref MethodNonDPCE
  String collocPtReuse;
  /// flag for usage of a filtered set of tensor-product grid points
  /// within regression PCE; based on the \c tensor_grid specification
  /// in \ref MethodNonDPCE
  bool probCollocFlag;
  /// the \c expansion_import_file specification in \ref MethodNonDPCE
  String expansionImportFile;
  /// the \c sample_type specification in \ref MethodNonDMC, \ref
  /// MethodNonDPCE, and \ref MethodNonDSC
  String sampleType;
  /// the type of limit state search in \ref MethodNonDLocalRel
  /// (\c x_taylor_mean, \c x_taylor_mpp, \c x_two_point, \c u_taylor_mean,
  /// \c u_taylor_mpp, \c u_two_point, or \c no_approx) or
  /// \ref MethodNonDGlobalRel (\c x_gaussian_process or \c u_gaussian_process)
  String reliabilitySearchType;
  /// the \c first_order or \c second_order integration selection in
  /// \ref MethodNonDLocalRel
  String reliabilityIntegration;
  /// the \c import, \c adapt_import, or \c mm_adapt_import integration
  /// refinement selection in \ref MethodNonDLocalRel, \ref MethodNonDPCE,
  /// and \ref MethodNonDSC
  String integrationRefine;
  /// the algorithm selection \c sqp or \c nip used for computing the MPP in
  /// \ref MethodNonDLocalRel or the interval in \ref MethodNonDLocalIntervalEst
  String nondOptAlgorithm;
  /// the \c distribution \c cumulative or \c complementary specification
  /// in \ref MethodNonDMC, \ref MethodNonDPCE, \ref MethodNonDLocalRel,
  /// and \ref MethodNonDGlobalRel
  String distributionType;
  /// the \c compute \c probabilities, \c reliabilities, or \c
  /// gen_reliabilities specification in \ref MethodNonDMC, \ref MethodNonDPCE,
  /// \ref MethodNonDLocalRel, and \ref MethodNonDGlobalRel
  String responseLevelMappingType;
  /// the \c response_levels specification in \ref MethodNonDMC,
  /// \ref MethodNonDPCE, \ref MethodNonDLocalRel, and \ref MethodNonDGlobalRel
  RealVectorArray responseLevels;
  /// the \c probability_levels specification in \ref MethodNonDMC,
  /// \ref MethodNonDPCE, \ref MethodNonDLocalRel, and \ref MethodNonDGlobalRel
  RealVectorArray probabilityLevels;
  /// the \c reliability_levels specification in \ref MethodNonDMC,
  /// \ref MethodNonDPCE, and \ref MethodNonDLocalRel
  RealVectorArray reliabilityLevels;
  /// the \c gen_reliability_levels specification in \ref MethodNonDMC,
  /// \ref MethodNonDPCE, \ref MethodNonDLocalRel, and \ref MethodNonDGlobalRel
  RealVectorArray genReliabilityLevels;
  /// the \c all_variables specification in \ref MethodNonDMC
  bool allVarsFlag;
  /// filename from which to read the x observed data for gpmsa
  String xObsDataFile;
  /// filename from which to read the x observed data for gpmsa
  String yObsDataFile;
  /// filename from which to read the y std of error for gpmsa
  String yStdDataFile;
  /// the number of samples to construct a GP emulator for Bayesian 
  /// calibration methods (\ref MethodNonDBayes)
  int emulatorSamples;
  /// the \c emulator specification in \ref MethodNonDBayes
  short emulatorType;
  /// the \c rejection type specification in \ref MethodNonDBayes
  String rejectionType;
  /// the \c metropolis type specification in \ref MethodNonDBayes
  String metropolisType;
  /// the \c proposal covariance scale factor in \ref MethodNonDBayes
  Real proposalCovScale;
  /// the \c likelihood scale factor in \ref MethodNonDBayes
  Real likelihoodScale;

  // Parameter Study

  /// the \c final_point specification in \ref MethodPSVPS
  RealVector finalPoint;
  /// the \c step_vector specification in \ref MethodPSVPS and \ref MethodPSCPS
  RealVector stepVector;
  /// the \c num_steps specification in \ref MethodPSVPS
  int numSteps;
  /// the \c deltas_per_variable specification in \ref MethodPSCPS
  IntVector stepsPerVariable;
  /// the \c list_of_points specification in \ref MethodPSLPS
  RealVector listOfPoints;
  /// the \c partitions specification for PStudy method in \ref MethodPSMPS
  UShortArray varPartitions;

  // Verification

  /// rate of mesh refinement in Richardson extrapolation
  Real refinementRate;

private:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataMethodRep();                             ///< constructor
  ~DataMethodRep();                            ///< destructor

  //
  //- Heading: Member methods
  //

  /// write a DataInterfaceRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataInterfaceRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataInterfaceRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing this dataMethodRep
  int referenceCount;
};


inline DataMethodRep::~DataMethodRep() { }


/// Handle class for method specification data.

/** The DataMethod class is used to provide a memory management handle
    for the data in DataMethodRep.  It is populated by
    IDRProblemDescDB::method_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A list of DataMethod
    objects is maintained in ProblemDescDB::dataMethodList, one for
    each method specification in an input file. */

class DataMethod
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;
  // library_mode default data initializer
  friend void run_dakota_data();

public:

  // comparison function
  static bool id_compare(const DataMethod& dm, const std::string& id)
  { return id == dm.dataMethodRep->idMethod; }

  //
  //- Heading: Constructors, destructor, operators
  //

  DataMethod();                                ///< constructor
  DataMethod(const DataMethod&);               ///< copy constructor
  ~DataMethod();                               ///< destructor

  DataMethod& operator=(const DataMethod&); ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// write a DataMethod object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataMethod object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);

  /// write a DataMethod object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataMethodRep* dataMethodRep;
};


/// MPIPackBuffer insertion operator for DataMethod
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataMethod& data)
{ data.write(s); return s; }


/// MPIUnpackBuffer extraction operator for DataMethod
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataMethod& data)
{ data.read(s); return s; }


/// std::ostream insertion operator for DataMethod
inline std::ostream& operator<<(std::ostream& s, const DataMethod& data)
{ data.write(s); return s; }

inline void DataMethod::write(std::ostream& s) const
{ dataMethodRep->write(s); }


inline void DataMethod::read(MPIUnpackBuffer& s)
{ dataMethodRep->read(s); }


inline void DataMethod::write(MPIPackBuffer& s) const
{ dataMethodRep->write(s); }

} // namespace Dakota

#endif
