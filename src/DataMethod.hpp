/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataMethod
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataMethod.hpp 6984 2010-09-27 02:11:09Z lpswile $

#ifndef DATA_METHOD_H
#define DATA_METHOD_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"

namespace Dakota {

// --------
// Iterator
// --------
// unsigned short 16 bits are apportioned as [Minimizer,Opt,LSq,SurrBased,
// Analyzer,NonD,PStudyDACE,Verif,Meta,...] where the trailing 6 bits are
// available for up to 64 methods within a particular classification.
#define    PARALLEL_BIT     64
#define        META_BIT    128
#define       VERIF_BIT    256
#define  PSTUDYDACE_BIT    512
#define        NOND_BIT   1024
#define    ANALYZER_BIT   2048
#define   SURRBASED_BIT   4096
#define     LEASTSQ_BIT   8192
#define   OPTIMIZER_BIT  16384
#define   MINIMIZER_BIT  32768
// define special values for method name.  Special bit selections enable 
// identification of method groupings based on the value.
enum { DEFAULT_METHOD=0,
       // Meta-Iterators:
       HYBRID=(META_BIT | PARALLEL_BIT), PARETO_SET, MULTI_START,
       //       BRANCH_AND_BOUND,
       // Verification Analyzers:
       RICHARDSON_EXTRAP=(ANALYZER_BIT | VERIF_BIT),
       // PStudyDACE Analyzers:
       CENTERED_PARAMETER_STUDY=(ANALYZER_BIT | PSTUDYDACE_BIT),
       LIST_PARAMETER_STUDY, MULTIDIM_PARAMETER_STUDY, VECTOR_PARAMETER_STUDY,
       DACE, FSU_CVT, FSU_HALTON, FSU_HAMMERSLEY, PSUADE_MOAT,
       // NonD Analyzers:
       LOCAL_RELIABILITY=(ANALYZER_BIT | NOND_BIT), GLOBAL_RELIABILITY,
       POLYNOMIAL_CHAOS, STOCH_COLLOCATION,
       CUBATURE_INTEGRATION, SPARSE_GRID_INTEGRATION, QUADRATURE_INTEGRATION, 
       BAYES_CALIBRATION, GPAIS, POF_DARTS, RKD_DARTS,
       IMPORTANCE_SAMPLING, ADAPTIVE_SAMPLING, MULTILEVEL_SAMPLING,
       LIST_SAMPLING, RANDOM_SAMPLING,
       // Variables::method_view(): epistemic if method_name > RANDOM_SAMPLING
       LOCAL_INTERVAL_EST, LOCAL_EVIDENCE, GLOBAL_INTERVAL_EST, GLOBAL_EVIDENCE,
       //BAYES_CALIBRATION=(ANALYZER_BIT | NOND_BIT | PARALLEL_BIT),
       //BRANCH_AND_BOUND=(MINIMIZER_BIT | PARALLEL_BIT),
       // SurrBased Minimizers:
       SURROGATE_BASED_LOCAL=(MINIMIZER_BIT | SURRBASED_BIT),
       DATA_FIT_SURROGATE_BASED_LOCAL, HIERARCH_SURROGATE_BASED_LOCAL,
       SURROGATE_BASED_GLOBAL, EFFICIENT_GLOBAL,
       // Gradient-based LeastSq Minimizers:
       NL2SOL=(MINIMIZER_BIT | LEASTSQ_BIT), NLSSOL_SQP, OPTPP_G_NEWTON,
       // Nongradient-based Optimizers / Minimizers:
       ASYNCH_PATTERN_SEARCH=(MINIMIZER_BIT | OPTIMIZER_BIT), OPTPP_PDS,
       COLINY_BETA, COLINY_COBYLA,         COLINY_DIRECT, COLINY_MULTI_START,
       COLINY_EA,   COLINY_PATTERN_SEARCH, COLINY_SOLIS_WETS,
       MOGA, SOGA, NCSU_DIRECT, MESH_ADAPTIVE_SEARCH, MIT_NOWPAC, MIT_SNOWPAC,
       GENIE_OPT_DARTS, GENIE_DIRECT,
       // Gradient-based Optimizers / Minimizers:
       NONLINEAR_CG, OPTPP_CG, OPTPP_Q_NEWTON, OPTPP_FD_NEWTON, OPTPP_NEWTON,
       NPSOL_SQP, NLPQL_SQP, //REDUCED_SQP,
       DOT_BFGS, DOT_FRCG, DOT_MMFD, DOT_SLP, DOT_SQP, CONMIN_FRCG, CONMIN_MFD,
       // Generic Optimizers / Minimizers:
       DL_SOLVER,
       // Minimizers that are both opt & least sq
       BRANCH_AND_BOUND=(MINIMIZER_BIT | OPTIMIZER_BIT | LEASTSQ_BIT) };

/// Sub-methods, including sampling, inference algorithm, opt algorithm types
enum { SUBMETHOD_DEFAULT=0, // no specification
       SUBMETHOD_NONE,      // spec override of default: no submethod
       /// Type of hybrid meta-iterator:
       SUBMETHOD_COLLABORATIVE,   SUBMETHOD_EMBEDDED,   SUBMETHOD_SEQUENTIAL,
       // Sampling and DOE types:
       SUBMETHOD_LHS,             SUBMETHOD_RANDOM,
       SUBMETHOD_BOX_BEHNKEN,     SUBMETHOD_CENTRAL_COMPOSITE,
       SUBMETHOD_GRID,            SUBMETHOD_OA_LHS,     SUBMETHOD_OAS,
       // Bayesian inference algorithms:
       SUBMETHOD_DREAM, SUBMETHOD_GPMSA, SUBMETHOD_QUESO, SUBMETHOD_WASABI,
       // optimization sub-method selections (in addition to SUBMETHOD_LHS):
       SUBMETHOD_NIP, SUBMETHOD_SQP, SUBMETHOD_EA, SUBMETHOD_EGO, SUBMETHOD_SBO,
       // verification approaches:
       SUBMETHOD_CONVERGE_ORDER,  SUBMETHOD_CONVERGE_QOI,
       SUBMETHOD_ESTIMATE_ORDER };

// define special values for outputLevel within
// Iterator/Model/Interface/Approximation
enum { SILENT_OUTPUT, QUIET_OUTPUT, NORMAL_OUTPUT, VERBOSE_OUTPUT,
       DEBUG_OUTPUT };
// define special values for Iterator and Interface scheduling
enum { DEFAULT_SCHEDULING, MASTER_SCHEDULING, PEER_SCHEDULING, 
       PEER_DYNAMIC_SCHEDULING, PEER_STATIC_SCHEDULING, DYNAMIC_SCHEDULING,
       STATIC_SCHEDULING };
// define special values for ParallelLibrary configuration logic
// related to scheduling
enum { DEFAULT_CONFIG, PUSH_DOWN, PUSH_UP };

// ----
// NonD
// ----
// define special values for u_space_type in
// NonD::initialize_random_variable_types()
enum { STD_NORMAL_U, STD_UNIFORM_U, ASKEY_U, EXTENDED_U };
// define special values for covarianceControl
enum { DEFAULT_COVARIANCE, NO_COVARIANCE, DIAGONAL_COVARIANCE,
       FULL_COVARIANCE };
// define special values for probability integration refinement
enum { NO_INT_REFINE=0, IS, AIS, MMAIS };
// define special values for responseLevelTarget
enum { PROBABILITIES, RELIABILITIES, GEN_RELIABILITIES };
// define special values for responseLevelTargetReduce
enum { COMPONENT=0, SYSTEM_SERIES, SYSTEM_PARALLEL };
// define special values for distributionType
enum { CUMULATIVE, COMPLEMENTARY };
// define special values for finalMomentsType
enum { NO_MOMENTS, STANDARD_MOMENTS, CENTRAL_MOMENTS };

// -------------
// NonDExpansion (most enums defined by Pecos in pecos_global_defs.hpp)
// -------------
// define special values for lsRegressionType
enum { DEFAULT_LS=0, SVD_LS, EQ_CON_LS };

// --------------------
// NonDBayesCalibration
// --------------------
// define special values for emulatorType
enum { NO_EMULATOR, PCE_EMULATOR, SC_EMULATOR, GP_EMULATOR, KRIGING_EMULATOR,
       VPS_EMULATOR };
// modes for calibrating multipliers on observational error
enum { CALIBRATE_NONE = 0, CALIBRATE_ONE, CALIBRATE_PER_EXPER, 
       CALIBRATE_PER_RESP, CALIBRATE_BOTH};

// ------------
// NonDSampling
// ------------
// LHS rank array processing modes:
enum { IGNORE_RANKS, SET_RANKS, GET_RANKS, SET_GET_RANKS };
// sampling modes (combinations of Uncertain/Active/All and Native/Uniform):
enum { UNCERTAIN,           UNCERTAIN_UNIFORM,
       ALEATORY_UNCERTAIN,  ALEATORY_UNCERTAIN_UNIFORM,
       EPISTEMIC_UNCERTAIN, EPISTEMIC_UNCERTAIN_UNIFORM,
       ACTIVE,              ACTIVE_UNIFORM,
       ALL,                 ALL_UNIFORM };
// (1) {,A,E}UNCERTAIN: sample only over the {,A,E} uncertain variables,
//     ignoring design/state, using the native distributions.
// (2) {,A,E}UNCERTAIN_UNIFORM: sample only over the {,A,E}uncertain variables,
//     ignoring design/state, using uniform distributions within native/inferred
//     bounds.
// (3) ACTIVE: sample only over the active variables, ignoring inactive, using
//     the native distributions (assumes uniform for design/state).
// (4) ACTIVE_UNIFORM: sample only over the active variables, ignoring inactive,
//     using uniform distributions within native/inferred bounds.
// (5) ALL: sample over All variables using native distributions (assumes
//     uniform for design/state).
// (6) ALL_UNIFORM: sample over All variables using uniform distributions.
// Note that ACTIVE modes will overlap with either UNCERTAIN or ALL modes
// depending on whether the Model/Variables employ an All or Distinct view.
// Wilks type of confidence interval
enum { ONE_SIDED_LOWER, ONE_SIDED_UPPER, TWO_SIDED };

// ---------------
// NonDReliability
// ---------------
// define special values for mppSearchType
enum { MV=0, AMV_X, AMV_U, AMV_PLUS_X, AMV_PLUS_U, TANA_X, TANA_U, NO_APPROX,
       EGRA_X, EGRA_U };
// define special values for secondOrderIntType
enum { BREITUNG, HOHENRACK, HONG };

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
  /// string to point to the low fidelity model for Bayesian experimental design
  String lowFidModelPointer;
  /// method verbosity control: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  /// (from the \c output specification in \ref MethodIndControl)
  short methodOutput;

  /// maximum number of iterations allowed for the method (from the \c
  /// max_iterations specification in \ref MethodIndControl)
  int maxIterations;
  /// maximum number of refinement iterations allowed for a uniform/adaptive
  /// refinement approach (from the \c max_refinement_iterations specification
  /// in \ref MethodIndControl)
  int maxRefineIterations;
  /// maximum number of internal solver iterations allowed for the method
  /// (from the \c max_solver_iterations specification in \ref MethodIndControl)
  int maxSolverIterations;
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

  /// the method selection: one of the optimizer, least squares, nond, dace,
  /// or parameter study methods
  unsigned short methodName;
  /// enum value for a sub-method type
  unsigned short subMethod;
  /// string identifier for a sub-method name within a multi-option
  /// method specification (e.g., from meta-iterators)
  String subMethodName;
  /// string pointer for a sub-model specification used by a meta-iterator
  String subModelPointer;
  /// string pointer for a sub-method specification used by a meta-iterator
  String subMethodPointer;

  /// number of servers for concurrent iterator parallelism (from
  /// the \c iterator_servers specification)
  int iteratorServers;
  /// number of processors for each concurrent iterator partition (from
  /// the \c processors_per_iterator specification)
  int procsPerIterator;
  /// type of scheduling ({DEFAULT,MASTER,PEER}_SCHEDULING) used in concurrent
  /// iterator parallelism (from the \c iterator_scheduling specification)
  short iteratorScheduling;

  /// array of methods for the sequential and collaborative hybrid
  /// meta-iterators (from the \c method_name_list specification)
  StringArray hybridMethodNames;
  /// array of models for the sequential and collaborative hybrid
  /// meta-iterators (from the \c model_pointer_list specification)
  StringArray hybridModelPointers;
  /// array of methods for the sequential and collaborative hybrid
  /// meta-iterators (from the \c method_pointer_list specification)
  StringArray hybridMethodPointers;
  // progress threshold for sequential adaptive hybrids (from the \c
  // progress_threshold specification)
  //Real hybridProgThresh;
  /// global method name for embedded hybrids (from the \c
  /// global_method_name specification)
  String hybridGlobalMethodName;
  /// global model pointer for embedded hybrids (from the \c
  /// global_model_pointer specification)
  String hybridGlobalModelPointer;
  /// global method pointer for embedded hybrids (from the \c
  /// global_method_pointer specification)
  String hybridGlobalMethodPointer;
  /// local method name for embedded hybrids (from the \c
  /// local_method_name specification)
  String hybridLocalMethodName;
  /// local model pointer for embedded hybrids (from the \c
  /// local_model_pointer specification)
  String hybridLocalModelPointer;
  /// local method pointer for embedded hybrids (from the \c
  /// local_method_pointer specification)
  String hybridLocalMethodPointer;
  /// local search probability for embedded hybrids (from the \c
  /// local_search_probability specification)
  Real hybridLSProb;

  /// number of random jobs to perform in the pareto_set and
  /// multi_start meta-iterators (from the \c random_starts and \c
  /// random_weight_sets specifications)
  int concurrentRandomJobs;
  /// user-specified (i.e., nonrandom) parameter sets to evaluate in
  /// the pareto_set and multi_start meta-iterators (from the \c
  /// starting_points and \c weight_sets specifications)
  RealVector concurrentParameterSets;

  /// number of consecutive iterations with change less than
  /// convergenceTolerance required to trigger convergence
  unsigned short softConvLimit;
  /// flag to indicate user-specification of a bypass of any/all
  /// layerings in evaluating truth response values in SBL.
  bool surrBasedLocalLayerBypass;
  /// initial trust region sizes in the surrogate-based local method
  /// (from the \c initial_size specification in \ref MethodSBL), one
  /// size per surrogate model (notes: no trust region for the truth
  /// model; sizes are relative values, e.g., 0.1 = 10% of range of
  /// global bounds for each variable
  RealVector trustRegionInitSize;
  /// minimum trust region size in the surrogate-based local method
  /// (from the \c minimum_size specification in \ref MethodSBL), if
  /// the trust region size falls below this threshold the SBL
  /// iterations are terminated (note: if kriging is used with SBL,
  /// the min trust region size is set to 1.0e-3 in attempt to avoid
  /// ill-conditioned matrixes that arise in kriging over small trust
  /// regions)
  Real trustRegionMinSize;
  /// trust region minimum improvement level (ratio of actual to predicted
  /// decrease in objective fcn) in the surrogate-based local method
  /// (from the \c contract_threshold specification in \ref MethodSBL),
  /// the trust region shrinks or is rejected if the ratio is below
  /// this value ("eta_1" in the Conn-Gould-Toint trust region book)
  Real trustRegionContractTrigger;
  /// trust region sufficient improvement level (ratio of actual to
  /// predicted decrease in objective fn) in the surrogate-based local
  /// method (from the \c expand_threshold specification in \ref
  /// MethodSBL), the trust region expands if the ratio is above this
  /// value ("eta_2" in the Conn-Gould-Toint trust region book)
  Real trustRegionExpandTrigger;
  /// trust region contraction factor in the surrogate-based local method
  /// (from the \c contraction_factor specification in \ref MethodSBL)
  Real trustRegionContract;
  /// trust region expansion factor in the surrogate-based local method
  /// (from the \c expansion_factor specification in \ref MethodSBL)
  Real trustRegionExpand;
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
  /// next surrogate is based in the \c surrogate_based_global method. 
  bool surrBasedGlobalReplacePts;

  // number of samples at the root for the branch and bound method
  // (from the \c num_samples_at_root specification in \ref MethodBandB)
  //int branchBndNumSamplesRoot;
  // number of samples at each node for the branch and bound method
  // (from the \c num_samples_at_node specification in \ref MethodBandB)
  //int branchBndNumSamplesNode;

  // DL_SOLVER

  /// string of options for a dynamically linked solver
  String dlDetails;
  /// handle to dynamically loaded library
  void *dlLib;

  // NPSOL

  /// the \c verify_level specification in \ref MethodNPSOLDC
  int verifyLevel;
  /// the \c function_precision specification in \ref MethodNPSOLDC
  /// and the \c EPSILON specification in NOMAD
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
  short meritFn;
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

  /// the initial \c constraint_penalty for COLINY methods in
  /// \ref MethodAPPS, \ref MethodSCOLIBDIR, \ref MethodSCOLIBPS,
  /// \ref MethodSCOLIBSW and \ref MethodSCOLIBEA
  Real constraintPenalty;
  /// the \c constant_penalty flag for COLINY methods in
  /// \ref MethodSCOLIBPS and \ref MethodSCOLIBSW
  bool constantPenalty;
  /// the \c global_balance_parameter for the DIRECT method in
  /// \ref MethodSCOLIBDIR
  Real globalBalanceParam;
  /// the \c local_balance_parameter for the DIRECT method in
  /// \ref MethodSCOLIBDIR
  Real localBalanceParam;
  /// the \c max_boxsize_limit for the DIRECT method in \ref MethodSCOLIBDIR
  Real maxBoxSize;
  /// the \c min_boxsize_limit for the DIRECT method in \ref MethodSCOLIBDIR
  /// and \ref MethodNCSUDC
  Real minBoxSize;
  /// the \c division setting (\c major_dimension or \c all_dimensions) for
  /// the DIRECT method in \ref MethodSCOLIBDIR
  String boxDivision;
  /// the \c non_adaptive specification for the coliny_ea method in
  /// \ref MethodSCOLIBEA
  bool mutationAdaptive;
  /// the \c show_misc_options specification in \ref MethodSCOLIBDC
  bool showMiscOptions;
  /// the \c misc_options specification in \ref MethodSCOLIBDC
  StringArray miscOptions;
  /// the \c solution_target specification in \ref MethodSCOLIBDC
  Real solnTarget;
  /// the \c crossover_rate specification for EA methods in \ref MethodSCOLIBEA
  Real crossoverRate;
  /// the \c mutation_rate specification for EA methods in \ref  MethodSCOLIBEA
  Real mutationRate;
  /// the \c mutation_scale specification for EA methods in \ref  MethodSCOLIBEA
  Real mutationScale;
  /// the \c min_scale specification for mutation in EA methods in
  /// \ref MethodSCOLIBEA
  Real mutationMinScale;
  /// the \c initial_delta specification for APPS/COBYLA/PS/SW methods in 
  /// \ref MethodAPPS, \ref MethodSCOLIBCOB, \ref MethodSCOLIBPS, and
  /// \ref MethodSCOLIBSW
  Real initDelta;
  /// the \c threshold_delta specification for APPS/COBYLA/PS/SW methods
  /// in \ref MethodAPPS, \ref MethodSCOLIBCOB, \ref MethodSCOLIBPS, and
  /// \ref MethodSCOLIBSW
  Real threshDelta;
  /// the \c contraction_factor specification for APPS/PS/SW methods in
  /// \ref MethodAPPS, \ref MethodSCOLIBPS, and \ref MethodSCOLIBSW
  Real contractFactor;
  /// the \c new_solutions_generated specification for GA/EPSA methods
  /// in \ref MethodSCOLIBEA
  int newSolnsGenerated;
  /// the integer assignment to random, chc, or elitist in the \c
  /// replacement_type specification for GA/EPSA methods in \ref
  /// MethodSCOLIBEA
  int numberRetained;
  /// the \c no_expansion specification for APPS/PS/SW methods in
  /// \ref MethodAPPS, \ref MethodSCOLIBPS, and \ref MethodSCOLIBSW
  bool expansionFlag;
  /// the \c expand_after_success specification for PS/SW methods in
  /// \ref MethodSCOLIBPS and \ref MethodSCOLIBSW
  int expandAfterSuccess;
  /// the \c contract_after_failure specification for the SW method in
  /// \ref MethodSCOLIBSW
  int contractAfterFail;
  /// the \c mutation_range specification for the pga_int method in
  /// \ref MethodSCOLIBEA
  int mutationRange;
  /// the \c total_pattern_size specification for PS methods in
  /// \ref MethodSCOLIBPS
  int totalPatternSize;
  /// the \c stochastic specification for the PS method in \ref MethodSCOLIBPS
  bool randomizeOrderFlag;
  /// the \c fitness_type specification for EA methods in \ref MethodSCOLIBEA
  String selectionPressure;
  /// the \c replacement_type specification for EA methods in
  /// \ref MethodSCOLIBEA
  String replacementType;
  /// the \c crossover_type specification for EA methods in \ref MethodSCOLIBEA
  String crossoverType;
  /// the \c mutation_type specification for EA methods in \ref MethodSCOLIBEA
  String mutationType;
  /// the \c exploratory_moves specification for the PS method in
  /// \ref MethodSCOLIBPS
  String exploratoryMoves;
  /// the \c pattern_basis specification for APPS/PS methods in
  /// \ref MethodAPPS and \ref MethodSCOLIBPS
  String patternBasis;
  /// beta solvers don't need documentation
  String betaSolverName;

  // COLINY and APPS

  /// the \c synchronization setting for parallel pattern search
  /// methods in \ref MethodSCOLIBPS and \ref MethodAPPS
  String evalSynchronize;

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
  // mainLoopType defined in JEGAOptimizer.cpp
  // evaluationType defined in JEGAOptimizer.cpp
  // delimiter for flat file read defined in JEGAOptimizer.cpp

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

  /// The maximum number of designs to keep when using the max_designs
  /// nicher
  size_t numDesigns;
  
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
  /// MethodSCOLIBEA
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

  // MADS
  /// the \c initMeshSize choice for NOMAD in \ref MethodNOMADDC
  Real initMeshSize;
  /// the \c minMeshSize choice for NOMAD in \ref MethodNOMADDC
  Real minMeshSize;
  /// the \c HISTORY_FILE specification for NOMAD
  String historyFile;
  /// the \c DISPLAY_STATS specification for NOMAD
  String displayFormat;
  /// the \c VNS specification for NOMAD
  Real vns;
  /// the \c NEIGHBOR_ORDER specification for NOMAD
  int neighborOrder;
  /// the \c DISPLAY_ALL_EVAL specification for NOMAD
  bool showAllEval;
  /// the \c HAS_SGTE specification for NOMAD
  String useSurrogate;

  // NonD & DACE

  /// the \c samples specification for NonD & DACE methods
  int numSamples;
  /// flag for fixing the value of the seed among different NonD/DACE
  /// sample sets.  This results in the use of the same sampling
  /// stencil/pattern throughout an execution with repeated sampling.
  bool fixedSeedFlag;
  /// flag for fixing the sequence for Halton or Hammersley QMC
  /// sample sets.  This results in the use of the same sampling
  /// stencil/pattern throughout an execution with repeated sampling.
  bool fixedSequenceFlag;
  /// the \c var_based_decomp specification for a variety of sampling methods
  bool vbdFlag;
  /// the \c var_based_decomp tolerance for omitting index output
  Real vbdDropTolerance;
  /// the \c backfill option allows one to augment in LHS sample 
  /// by enforcing the addition of unique discrete variables to the sample
  bool backfillFlag;
  /// Flag to specify the calculation of principal components when 
  /// using LHS  
  bool pcaFlag;
  /// The percentage of variance explained by using a truncated 
  /// number of principal components in PCA
  Real percentVarianceExplained;
  /// Flag to specify use of Wilks formula to calculate num samples
  bool wilksFlag;
  /// Wilks order parameter
  unsigned short wilksOrder;
  /// Wilks confidence interval parameter
  Real wilksConfidenceLevel;
  /// Wilks sided interval type
  short wilksSidedInterval;

  // NonD

  /// a sub-specification of vbdFlag: interaction order limit for
  /// calculation/output of component VBD indices
  unsigned short vbdOrder;
  /// restrict the calculation of a full response covariance matrix
  /// for high dimensional outputs: {DEFAULT,DIAGONAL,FULL}_COVARIANCE
  short covarianceControl;
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
  /// enumeration for u-space type that defines u-space variable targets
  /// for probability space transformations: EXTENDED_U (default), ASKEY_U,
  /// STD_NORMAL_U, or STD_UNIFORM_U
  short expansionType;
  /// boolean indicating presence of \c piecewise keyword
  bool piecewiseBasis;
  /// enumeration for type of basis in sparse grid interpolation
  /// (Pecos::{NODAL,HIERARCHICAL}_INTERPOLANT) or regression
  /// (Pecos::{TENSOR_PRODUCT,TOTAL_ORDER,ADAPTED}_BASIS).
  short expansionBasisType;
  /// the \c expansion_order specification in \ref MethodNonDPCE
  UShortArray expansionOrder;
  /// the \c expansion_samples specification in \ref MethodNonDPCE
  SizetArray expansionSamples;
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
  /// and expansion orders in \ref MethodNonDPCE and \ref MethodNonDSC
  RealVector anisoDimPref;
  /// the \c cubature_integrand specification in \ref MethodNonDPCE
  unsigned short cubIntOrder;
  /// the \c collocation_points specification in \ref MethodNonDPCE
  SizetArray collocationPoints;
  /// the \c collocation_ratio specification in \ref MethodNonDPCE
  Real collocationRatio;
  /// order applied to the number of expansion terms when applying
  /// or computing the collocation ratio within regression PCE;
  /// based on the \c ratio_order specification in \ref MethodNonDPCE
  Real collocRatioTermsOrder;
  /// type of regression: LS, OMP, BP, BPDN, LARS, or LASSO
  short regressionType;
  /// type of least squares regression: SVD or EQ_CON_QR
  short lsRegressionType;
  /// noise tolerance(s) for OMP, BPDN, LARS, and LASSO
  RealVector regressionNoiseTol;
  /// L2 regression penalty for a variant of LASSO known as the
  /// elastic net method (default of 0 gives standard LASSO)
  Real regressionL2Penalty;
  /// flag indicating the use of cross-validation across expansion orders
  /// (given a prescribed maximum order) and, for some methods, noise tolerances
  bool crossValidation;
  /// flag indicating the restriction of cross-validation to estimate only
  /// the most effective noise tolerance; used to reduce cost from performing
  /// CV over both noise tolerances and expansion orders
  bool crossValidNoiseOnly;
  // initial grid level for the ADAPTED_BASIS_GENERALIZED approach to
  // defining the candidate basis for sparse recovery (compressed sensing)
  //unsigned short adaptedBasisInitLevel;
  /// initial grid level for the ADAPTED_BASIS_EXPANDING_FRONT approach to
  /// defining the candidate basis for sparse recovery (compressed sensing)
  unsigned short adaptedBasisAdvancements;
  /// flag indicating the output of PCE coefficients corresponding to
  /// normalized basis polynomials
  bool normalizedCoeffs;
  /// allows PCE construction to reuse points from previous sample
  /// sets or data import using the \c reuse_points specification in
  /// \ref MethodNonDPCE
  String pointReuse;
  /// flag for usage of a sub-sampled set of tensor-product grid points
  /// within regression PCE; based on the \c tensor_grid specification
  /// in \ref MethodNonDPCE
  bool tensorGridFlag;
  /// order of tensor-product grid points that are sub-sampled within
  /// orthogonal least interpolation PCE; based on the \c tensor_grid
  /// specification in \ref MethodNonDPCE
  UShortArray tensorGridOrder;
  /// the \c import_expansion_file specification in \ref MethodNonDPCE
  String importExpansionFile;
  /// the \c export_expansion_file specification in \ref MethodNonDPCE
  String exportExpansionFile;
  /// the \c sample_type specification in \ref MethodNonDMC, \ref
  /// MethodNonDPCE, and \ref MethodNonDSC
  unsigned short sampleType;
  /// whether to generate D-optimal designs
  bool dOptimal;
  /// number of candidate designss in D-optimal design selection
  size_t numCandidateDesigns;
  /// the type of limit state search in \ref MethodNonDLocalRel
  /// (\c x_taylor_mean, \c x_taylor_mpp, \c x_two_point, \c u_taylor_mean,
  /// \c u_taylor_mpp, \c u_two_point, or \c no_approx) or
  /// \ref MethodNonDGlobalRel (\c x_gaussian_process or \c u_gaussian_process)
  unsigned short reliabilitySearchType;
  /// the \c first_order or \c second_order integration selection in
  /// \ref MethodNonDLocalRel
  String reliabilityIntegration;
  /// the \c import, \c adapt_import, or \c mm_adapt_import integration
  /// refinement selection in \ref MethodNonDLocalRel, \ref MethodNonDPCE,
  /// and \ref MethodNonDSC
  unsigned short integrationRefine;
  /// Sequence of refinement samples, e.g., the size of the batch
  /// (e.g. number of supplemental points added) to be added to be
  /// added to the build points for an emulator at each iteration
  IntVector refineSamples;
  /// the \c pilot_samples selection in \ref MethodMultilevelMC
  SizetArray pilotSamples;
  /// the \c final_moments specification in \ref MethodNonD
  short finalMomentsType;
  /// the \c distribution \c cumulative or \c complementary specification
  /// in \ref MethodNonD
  short distributionType;
  /// the \c compute \c probabilities, \c reliabilities, or \c
  /// gen_reliabilities specification in \ref MethodNonD
  short responseLevelTarget;
  /// the \c system \c series or \c parallel specification in \ref MethodNonD
  short responseLevelTargetReduce;
  /// the \c response_levels specification in \ref MethodNonD
  RealVectorArray responseLevels;
  /// the \c probability_levels specification in \ref MethodNonD
  RealVectorArray probabilityLevels;
  /// the \c reliability_levels specification in \ref MethodNonD
  RealVectorArray reliabilityLevels;
  /// the \c gen_reliability_levels specification in \ref MethodNonD
  RealVectorArray genReliabilityLevels;
  /// the number of MCMC chain samples
  int chainSamples;
  /// the number of samples to construct an emulator, e.g., for
  /// Bayesian calibration methods
  int buildSamples;
  /// number of samples to perform on emulator
  int samplesOnEmulator;
  /// The total order to be used in construction of a VPS surrogate 
  int emulatorOrder;
  /// the \c emulator specification in \ref MethodNonDBayesCalib
  short emulatorType;
  /// the \c mcmc type specification in \ref MethodNonDBayesCalib
  String mcmcType;
  /// use of standardized probability spaces for MCMC within Bayesian inference
  bool standardizedSpace;
  /// flag indicating adaptive refinement of the emulator in regions
  /// of high posterior probability
  bool adaptPosteriorRefine;
  /// flag indicating user activation of logit transform option within QUESO
  bool logitTransform;
  /// whether to apply GPMSA-internal normalization
  bool gpmsaNormalize;
  /// flag indicating the calculation of KL divergence between prior
  /// and posterior in Bayesian methods 
  bool posteriorStatsKL;
  /// flag indicating the calculation of mutual information between prior
  /// and posterior in Bayesian methods 
  bool posteriorStatsMutual;
  /// the method used for performing a pre-solve for the MAP point
  unsigned short preSolveMethod;
  /// the type of proposal covariance: user, derivatives, or prior
  String proposalCovType;
  /// number of updates of the proposal covariance from computing the
  /// misfit Hessian using residual values and derivatives
  int proposalCovUpdates;
  /// the format of proposal covariance input: diagonal or matrix
  String proposalCovInputType;
  /// raw list of real data for the proposal covariance
  RealVector proposalCovData;
  /// file from which to read proposal covariance in diagonal or matrix format
  String proposalCovFile;
  /// file containing advanced QUESO option overrides
  String quesoOptionsFilename;
  /// the \c fitness metric type specification in \ref
  /// MethodNonDAdaptive
  String fitnessMetricType;
  /// the \c batch selection type specification in \ref
  /// MethodNonDAdaptive
  String batchSelectionType;
  /// the \c Lipschitz type specification in \ref
  /// MethodNonDPOFDarts (e.g. either local or global estimation)
  String lipschitzType;
  /// calibration mode for observation error multipliers (CALIBRATE_*)
  unsigned short calibrateErrorMode;
  /// hyperparameters inverse gamma prior alphas
  RealVector hyperPriorAlphas;
  /// hyperparameters inverse gamma prior alphas
  RealVector hyperPriorBetas;
  /// number of MCMC samples to discard from acceptance chain
  int burnInSamples;
  /// period or skip in post-processing the acceptance chain
  int subSamplingPeriod;
  /// flag to calculate model discrepancy
  bool calModelDiscrepancy;
  /// number of prediction configurations at which to calculate model 
  /// discrepancy
  size_t numPredConfigs;
  /// list of prediction configurations at which to calculate model discrepancy
  RealVector predictionConfigList;
  /// whether to import prediction configurations at which to calculate model
  /// discrepancy
  String importPredConfigs;
  /// tabular format for prediction configurations import file
  unsigned short importPredConfigFormat;
  /// specify type of model discrepancy formulation
  String discrepancyType;
  /// correction order for either gaussian process or polynomial model
  /// discrepancy calculations: 0 (=constant), 1 (=linear), 2 (=quadratic)
  short approxCorrectionOrder;
  /// specify the name of file to which corrected model (model+discrepancy)
  /// calculations are output
  String exportCorrModelFile;
  /// tabular format for corrected model (model+discrepancy) export file
  unsigned short exportCorrModelFormat;
  /// specify the name of file to which corrected model variance
  /// calculations are output
  String exportCorrVarFile;
  /// tabular format for corrected model variance export file
  unsigned short exportCorrVarFormat;
  /// specify the name of file to which discrepancy calculations are output
  String exportDiscrepFile;
  /// tabular format for model discrepancy export file
  unsigned short exportDiscrepFormat;
  /// whether to perform adaptive Bayesian design of experiments
  bool adaptExpDesign;
  /// whether to import candidate design points for adaptive Bayesian experimtal
  /// design
  String importCandPtsFile;
  /// tabular format for the candidate design points import file
  unsigned short importCandFormat;
  /// number of candidate designs for adaptive Bayesian experimental design
  size_t numCandidates;
  /// maximum number of highfidelity model runs to be used for adaptive Bayesian 
  /// experimental design
  int maxHifiEvals;

  // DREAM sub-specification

  /// number of concurrent chains
  int numChains;
  /// number of CR-factors
  int numCR;
  /// number of crossover chain pairs
  int crossoverChainPairs;
  /// threshold for the Gelmin-Rubin statistic
  Real grThreshold;
  /// how often to perform a long jump in generations
  int jumpStep; 

  // WASABI sub-specification

  /// the type of data distribution: kde, or gaussian
  String dataDistType;
  /// the format of data distribution gaussian covariance input: 
  /// diagonal or matrix
  String dataDistCovInputType;
  /// raw list of real data for the data distribution gaussian means
  RealVector dataDistMeans;
  /// raw list of real data for the data distribution gaussian covariance
  RealVector dataDistCovariance;
  /// file from which to read data distribution data (covariance or samples )
  String dataDistFile;
  /// The filename of the export file containing an arbitrary set of samples and 
  /// their corresponding density values
  String posteriorDensityExportFilename;
  /// The filename of the export file containing samples from the posterior and 
  /// their corresponding density values
  String posteriorSamplesExportFilename;
  /// The filename of the import file containing samples at which the 
  /// posterior will be evaluated
  String posteriorSamplesImportFilename;
  /// Flag specifying whether to generate random samples from the posterior
  bool generatePosteriorSamples;
  /// Flag specifying whether to evaluate the posterior density at a 
  /// set of samples
  bool evaluatePosteriorDensity;

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
  /// the \c import_points_file spec for a file-based parameter study
  String pstudyFilename;
  /// tabular format for the parameter study points file
  unsigned short pstudyFileFormat;
  /// whether to import active variables only
  bool pstudyFileActive;
  /// the \c partitions specification for PStudy method in \ref MethodPSMPS
  UShortArray varPartitions;

  // Verification

  /// rate of mesh refinement in Richardson extrapolation
  Real refinementRate;
 
  // File read for surrogates

  /// the file name from the \c import_build_points_file specification
  String importBuildPtsFile;
  /// tabular format for the build point import file
  unsigned short importBuildFormat;
  /// whether to import active variables only
  bool importBuildActive;

  /// the file name from the \c import_approx_points_file specification
  String importApproxPtsFile;
  /// tabular format for the approx point import file
  unsigned short importApproxFormat;
  /// whether to import active variables only
  bool importApproxActive;

  /// the file name from the \c export_approx_points_file specification
  String exportApproxPtsFile;
  /// tabular format for the approx point export file
  unsigned short exportApproxFormat;

  /// the file name from the \c export_mcmc_points_file specification
  String exportMCMCPtsFile;
  /// flag for exporting the sequence of sample increments within
  /// multilevel sampling from the \c export_sample_sequence specification
  bool exportSampleSeqFlag;
  /// tabular format for the MCMC chain and MLMC sample sequence exports
  unsigned short exportSamplesFormat;

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

public:

  /// compares the idMethod attribute of DataMethod objects
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

  /// return dataMethodRep
  DataMethodRep* data_rep();

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataMethodRep* dataMethodRep;
};


inline DataMethodRep* DataMethod::data_rep()
{return dataMethodRep; }


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
