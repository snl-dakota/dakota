#pragma once

#include "dakota_global_defs.hpp"
#include "pecos_global_defs.hpp"

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
       EXTERNAL_PYTHON,
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
       SURROGATE_BASED_UQ, POLYNOMIAL_CHAOS, MULTILEVEL_POLYNOMIAL_CHAOS,
       MULTIFIDELITY_POLYNOMIAL_CHAOS, STOCH_COLLOCATION,
       MULTIFIDELITY_STOCH_COLLOCATION, C3_FUNCTION_TRAIN,
       MULTILEVEL_FUNCTION_TRAIN, MULTIFIDELITY_FUNCTION_TRAIN,
       CUBATURE_INTEGRATION, SPARSE_GRID_INTEGRATION, QUADRATURE_INTEGRATION, 
       BAYES_CALIBRATION, GPAIS, POF_DARTS, RKD_DARTS,
       IMPORTANCE_SAMPLING, ADAPTIVE_SAMPLING,
       MULTILEVEL_SAMPLING, MULTIFIDELITY_SAMPLING,
       MULTILEVEL_MULTIFIDELITY_SAMPLING, APPROX_CONTROL_VARIATE,
       GEN_APPROX_CONTROL_VARIATE, MULTILEVEL_BLUE,
       LIST_SAMPLING, RANDOM_SAMPLING,
       IMPORT_POINTS, // TNP TODO: Maybe move this depending on where it goes in the class hierarchy
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
       // Place Demo Opt TPL here based on current state of non-gradient flavor
       DEMO_TPL,
       // Gradient-based Optimizers / Minimizers:
       NONLINEAR_CG, OPTPP_CG, OPTPP_Q_NEWTON, OPTPP_FD_NEWTON, OPTPP_NEWTON,
       NPSOL_SQP, NLPQL_SQP, //REDUCED_SQP,
       DOT_BFGS, DOT_FRCG, DOT_MMFD, DOT_SLP, DOT_SQP, CONMIN_FRCG, CONMIN_MFD,
       ROL,
       // Generic Optimizers / Minimizers:
       DL_SOLVER,
       // Minimizers that are both opt & least sq
       BRANCH_AND_BOUND=(MINIMIZER_BIT | OPTIMIZER_BIT | LEASTSQ_BIT) };

// The use of composite enums necessitates manual registry of the corresponding string map
REGISTER_DAKOTA_ENUM( DEFAULT_METHOD,                     DEFAULT_METHOD)
REGISTER_DAKOTA_ENUM( EXTERNAL_PYTHON,                    EXTERNAL_PYTHON)
REGISTER_DAKOTA_ENUM( HYBRID,                             HYBRID)
REGISTER_DAKOTA_ENUM( PARETO_SET,                         PARETO_SET)
REGISTER_DAKOTA_ENUM( MULTI_START,                        MULTI_START)
REGISTER_DAKOTA_ENUM( RICHARDSON_EXTRAP,                  RICHARDSON_EXTRAP)
REGISTER_DAKOTA_ENUM( CENTERED_PARAMETER_STUDY,           CENTERED_PARAMETER_STUDY)
REGISTER_DAKOTA_ENUM( LIST_PARAMETER_STUDY,               LIST_PARAMETER_STUDY)
REGISTER_DAKOTA_ENUM( MULTIDIM_PARAMETER_STUDY,           MULTIDIM_PARAMETER_STUDY)
REGISTER_DAKOTA_ENUM( VECTOR_PARAMETER_STUDY,             VECTOR_PARAMETER_STUDY)
REGISTER_DAKOTA_ENUM( DACE,                               DACE)
REGISTER_DAKOTA_ENUM( FSU_CVT,                            FSU_CVT)
REGISTER_DAKOTA_ENUM( FSU_HALTON,                         FSU_HALTON)
REGISTER_DAKOTA_ENUM( FSU_HAMMERSLEY,                     FSU_HAMMERSLEY)
REGISTER_DAKOTA_ENUM( PSUADE_MOAT,                        PSUADE_MOAT)
REGISTER_DAKOTA_ENUM( LOCAL_RELIABILITY,                  LOCAL_RELIABILITY)
REGISTER_DAKOTA_ENUM( GLOBAL_RELIABILITY,                 GLOBAL_RELIABILITY)
REGISTER_DAKOTA_ENUM( SURROGATE_BASED_UQ,                 SURROGATE_BASED_UQ)
REGISTER_DAKOTA_ENUM( POLYNOMIAL_CHAOS,                   POLYNOMIAL_CHAOS)
REGISTER_DAKOTA_ENUM( MULTILEVEL_POLYNOMIAL_CHAOS,        MULTILEVEL_POLYNOMIAL_CHAOS)
REGISTER_DAKOTA_ENUM( MULTIFIDELITY_POLYNOMIAL_CHAOS,     MULTIFIDELITY_POLYNOMIAL_CHAOS)
REGISTER_DAKOTA_ENUM( STOCH_COLLOCATION,                  STOCH_COLLOCATION)
REGISTER_DAKOTA_ENUM( MULTIFIDELITY_STOCH_COLLOCATION,    MULTIFIDELITY_STOCH_COLLOCATION)
REGISTER_DAKOTA_ENUM( C3_FUNCTION_TRAIN,                  C3_FUNCTION_TRAIN)
REGISTER_DAKOTA_ENUM( MULTILEVEL_FUNCTION_TRAIN,          MULTILEVEL_FUNCTION_TRAIN)
REGISTER_DAKOTA_ENUM( MULTIFIDELITY_FUNCTION_TRAIN,       MULTIFIDELITY_FUNCTION_TRAIN)
REGISTER_DAKOTA_ENUM( CUBATURE_INTEGRATION,               CUBATURE_INTEGRATION)
REGISTER_DAKOTA_ENUM( SPARSE_GRID_INTEGRATION,            SPARSE_GRID_INTEGRATION)
REGISTER_DAKOTA_ENUM( QUADRATURE_INTEGRATION,             QUADRATURE_INTEGRATION)
REGISTER_DAKOTA_ENUM( BAYES_CALIBRATION,                  BAYES_CALIBRATION)
REGISTER_DAKOTA_ENUM( GPAIS,                              GPAIS)
REGISTER_DAKOTA_ENUM( POF_DARTS,                          POF_DARTS)
REGISTER_DAKOTA_ENUM( RKD_DARTS,                          RKD_DARTS)
REGISTER_DAKOTA_ENUM( IMPORTANCE_SAMPLING,                IMPORTANCE_SAMPLING)
REGISTER_DAKOTA_ENUM( ADAPTIVE_SAMPLING,                  ADAPTIVE_SAMPLING)
REGISTER_DAKOTA_ENUM( MULTILEVEL_SAMPLING,                MULTILEVEL_SAMPLING)
REGISTER_DAKOTA_ENUM( MULTIFIDELITY_SAMPLING,             MULTIFIDELITY_SAMPLING)
REGISTER_DAKOTA_ENUM( MULTILEVEL_MULTIFIDELITY_SAMPLING,  MULTILEVEL_MULTIFIDELITY_SAMPLING)
REGISTER_DAKOTA_ENUM( APPROX_CONTROL_VARIATE,             APPROX_CONTROL_VARIATE)
REGISTER_DAKOTA_ENUM( GEN_APPROX_CONTROL_VARIATE,         GEN_APPROX_CONTROL_VARIATE)
REGISTER_DAKOTA_ENUM( MULTILEVEL_BLUE,                    MULTILEVEL_BLUE)
REGISTER_DAKOTA_ENUM( LIST_SAMPLING,                      LIST_SAMPLING)
REGISTER_DAKOTA_ENUM( RANDOM_SAMPLING,                    RANDOM_SAMPLING)
REGISTER_DAKOTA_ENUM( IMPORT_POINTS,                      IMPORT_POINTS)
REGISTER_DAKOTA_ENUM( LOCAL_INTERVAL_EST,                 LOCAL_INTERVAL_EST)
REGISTER_DAKOTA_ENUM( LOCAL_EVIDENCE,                     LOCAL_EVIDENCE)
REGISTER_DAKOTA_ENUM( GLOBAL_INTERVAL_EST,                GLOBAL_INTERVAL_EST)
REGISTER_DAKOTA_ENUM( GLOBAL_EVIDENCE,                    GLOBAL_EVIDENCE)
REGISTER_DAKOTA_ENUM( SURROGATE_BASED_LOCAL,              SURROGATE_BASED_LOCAL)
REGISTER_DAKOTA_ENUM( DATA_FIT_SURROGATE_BASED_LOCAL,     DATA_FIT_SURROGATE_BASED_LOCAL)
REGISTER_DAKOTA_ENUM( HIERARCH_SURROGATE_BASED_LOCAL,     HIERARCH_SURROGATE_BASED_LOCAL)
REGISTER_DAKOTA_ENUM( SURROGATE_BASED_GLOBAL,             SURROGATE_BASED_GLOBAL)
REGISTER_DAKOTA_ENUM( EFFICIENT_GLOBAL,                   EFFICIENT_GLOBAL)
REGISTER_DAKOTA_ENUM( NL2SOL,                             NL2SOL)
REGISTER_DAKOTA_ENUM( NLSSOL_SQP,                         NLSSOL_SQP)
REGISTER_DAKOTA_ENUM( OPTPP_G_NEWTON,                     OPTPP_G_NEWTON)
REGISTER_DAKOTA_ENUM( ASYNCH_PATTERN_SEARCH,              ASYNCH_PATTERN_SEARCH)
REGISTER_DAKOTA_ENUM( OPTPP_PDS,                          OPTPP_PDS)
REGISTER_DAKOTA_ENUM( COLINY_BETA,                        COLINY_BETA)
REGISTER_DAKOTA_ENUM( COLINY_COBYLA,                      COLINY_COBYLA)
REGISTER_DAKOTA_ENUM( COLINY_DIRECT,                      COLINY_DIRECT)
REGISTER_DAKOTA_ENUM( COLINY_MULTI_START,                 COLINY_MULTI_START)
REGISTER_DAKOTA_ENUM( COLINY_EA,                          COLINY_EA)
REGISTER_DAKOTA_ENUM( COLINY_PATTERN_SEARCH,              COLINY_PATTERN_SEARCH)
REGISTER_DAKOTA_ENUM( COLINY_SOLIS_WETS,                  COLINY_SOLIS_WETS)
REGISTER_DAKOTA_ENUM( MOGA,                               MOGA)
REGISTER_DAKOTA_ENUM( SOGA,                               SOGA)
REGISTER_DAKOTA_ENUM( NCSU_DIRECT,                        NCSU_DIRECT)
REGISTER_DAKOTA_ENUM( MESH_ADAPTIVE_SEARCH,               MESH_ADAPTIVE_SEARCH)
REGISTER_DAKOTA_ENUM( MIT_NOWPAC,                         MIT_NOWPAC)
REGISTER_DAKOTA_ENUM( MIT_SNOWPAC,                        MIT_SNOWPAC)
REGISTER_DAKOTA_ENUM( GENIE_OPT_DARTS,                    GENIE_OPT_DARTS)
REGISTER_DAKOTA_ENUM( GENIE_DIRECT,                       GENIE_DIRECT)
REGISTER_DAKOTA_ENUM( DEMO_TPL,                           DEMO_TPL)
REGISTER_DAKOTA_ENUM( NONLINEAR_CG,                       NONLINEAR_CG)
REGISTER_DAKOTA_ENUM( OPTPP_CG,                           OPTPP_CG)
REGISTER_DAKOTA_ENUM( OPTPP_Q_NEWTON,                     OPTPP_Q_NEWTON)
REGISTER_DAKOTA_ENUM( OPTPP_FD_NEWTON,                    OPTPP_FD_NEWTON)
REGISTER_DAKOTA_ENUM( OPTPP_NEWTON,                       OPTPP_NEWTON)
REGISTER_DAKOTA_ENUM( NPSOL_SQP,                          NPSOL_SQP)
REGISTER_DAKOTA_ENUM( NLPQL_SQP,                          NLPQL_SQP)
REGISTER_DAKOTA_ENUM( DOT_BFGS,                           DOT_BFGS)
REGISTER_DAKOTA_ENUM( DOT_FRCG,                           DOT_FRCG)
REGISTER_DAKOTA_ENUM( DOT_MMFD,                           DOT_MMFD)
REGISTER_DAKOTA_ENUM( DOT_SLP,                            DOT_SLP)
REGISTER_DAKOTA_ENUM( DOT_SQP,                            DOT_SQP)
REGISTER_DAKOTA_ENUM( CONMIN_FRCG,                        CONMIN_FRCG)
REGISTER_DAKOTA_ENUM( CONMIN_MFD,                         CONMIN_MFD)
REGISTER_DAKOTA_ENUM( ROL,                                ROL)
REGISTER_DAKOTA_ENUM( DL_SOLVER,                          DL_SOLVER)
REGISTER_DAKOTA_ENUM( BRANCH_AND_BOUND,                   BRANCH_AND_BOUND)

/// Sub-methods, including sampling, inference algorithm, opt algorithm types
enum { SUBMETHOD_DEFAULT=0, // no specification
       SUBMETHOD_NONE,      // spec override of default: no submethod
       /// Type of hybrid meta-iterator:
       SUBMETHOD_COLLABORATIVE,   SUBMETHOD_EMBEDDED,   SUBMETHOD_SEQUENTIAL,
       // Sampling and DOE types:
       SUBMETHOD_LHS,                      SUBMETHOD_RANDOM,
       SUBMETHOD_LOW_DISCREPANCY_SAMPLING, SUBMETHOD_BOX_BEHNKEN,
       SUBMETHOD_CENTRAL_COMPOSITE,        SUBMETHOD_GRID,
       SUBMETHOD_OA_LHS,                   SUBMETHOD_OAS,
       // ACV modes and options for "promotions" for MFMC,MLMC:
       SUBMETHOD_MFMC,   SUBMETHOD_WEIGHTED_MLMC,
       SUBMETHOD_ACV_IS, SUBMETHOD_ACV_MF, SUBMETHOD_ACV_RD,
       // Bayesian inference algorithms:
       SUBMETHOD_DREAM, SUBMETHOD_GPMSA, SUBMETHOD_MUQ, SUBMETHOD_QUESO,
       SUBMETHOD_WASABI,
       // optimization sub-method selections (in addition to SUBMETHOD_LHS):
       SUBMETHOD_CONMIN, SUBMETHOD_DOT, SUBMETHOD_NLPQL, SUBMETHOD_NPSOL,
       SUBMETHOD_OPTPP, SUBMETHOD_NPSOL_OPTPP,
       SUBMETHOD_DIRECT, SUBMETHOD_DIRECT_NPSOL_OPTPP,
       SUBMETHOD_DIRECT_NPSOL, SUBMETHOD_DIRECT_OPTPP, 
       SUBMETHOD_EA, SUBMETHOD_EGO, SUBMETHOD_SBLO, SUBMETHOD_SBGO,
       // Local reliability sub-method selections: (MV is 0)
       SUBMETHOD_AMV_X,       SUBMETHOD_AMV_U,
       SUBMETHOD_AMV_PLUS_X,  SUBMETHOD_AMV_PLUS_U,
       SUBMETHOD_TANA_X,      SUBMETHOD_TANA_U,
       SUBMETHOD_QMEA_X,      SUBMETHOD_QMEA_U,
       SUBMETHOD_NO_APPROX,
       // Global reliability sub-method selections:
       SUBMETHOD_EGRA_X,      SUBMETHOD_EGRA_U,
       // verification approaches:
       SUBMETHOD_CONVERGE_ORDER,  SUBMETHOD_CONVERGE_QOI,
       SUBMETHOD_ESTIMATE_ORDER };
REGISTER_DAKOTA_ENUM( SUBMETHOD_DEFAULT,                  SUBMETHOD_DEFAULT)
REGISTER_DAKOTA_ENUM( SUBMETHOD_NONE,                     SUBMETHOD_NONE)
REGISTER_DAKOTA_ENUM( SUBMETHOD_COLLABORATIVE ,           SUBMETHOD_COLLABORATIVE )
REGISTER_DAKOTA_ENUM( SUBMETHOD_EMBEDDED ,                SUBMETHOD_EMBEDDED )
REGISTER_DAKOTA_ENUM( SUBMETHOD_SEQUENTIAL,               SUBMETHOD_SEQUENTIAL)
REGISTER_DAKOTA_ENUM( SUBMETHOD_LHS ,                     SUBMETHOD_LHS )
REGISTER_DAKOTA_ENUM( SUBMETHOD_RANDOM,                   SUBMETHOD_RANDOM)
REGISTER_DAKOTA_ENUM( SUBMETHOD_LOW_DISCREPANCY_SAMPLING, SUBMETHOD_LOW_DISCREPANCY_SAMPLING)
REGISTER_DAKOTA_ENUM( SUBMETHOD_BOX_BEHNKEN,              SUBMETHOD_BOX_BEHNKEN)
REGISTER_DAKOTA_ENUM( SUBMETHOD_CENTRAL_COMPOSITE ,       SUBMETHOD_CENTRAL_COMPOSITE )
REGISTER_DAKOTA_ENUM( SUBMETHOD_GRID,                     SUBMETHOD_GRID)
REGISTER_DAKOTA_ENUM( SUBMETHOD_OA_LHS ,                  SUBMETHOD_OA_LHS )
REGISTER_DAKOTA_ENUM( SUBMETHOD_OAS,                      SUBMETHOD_OAS)
REGISTER_DAKOTA_ENUM( SUBMETHOD_MFMC ,                    SUBMETHOD_MFMC )
REGISTER_DAKOTA_ENUM( SUBMETHOD_WEIGHTED_MLMC,            SUBMETHOD_WEIGHTED_MLMC)
REGISTER_DAKOTA_ENUM( SUBMETHOD_ACV_IS ,                  SUBMETHOD_ACV_IS )
REGISTER_DAKOTA_ENUM( SUBMETHOD_ACV_MF ,                  SUBMETHOD_ACV_MF )
REGISTER_DAKOTA_ENUM( SUBMETHOD_ACV_RD,                   SUBMETHOD_ACV_RD)
REGISTER_DAKOTA_ENUM( SUBMETHOD_DREAM ,                   SUBMETHOD_DREAM )
REGISTER_DAKOTA_ENUM( SUBMETHOD_GPMSA ,                   SUBMETHOD_GPMSA )
REGISTER_DAKOTA_ENUM( SUBMETHOD_MUQ ,                     SUBMETHOD_MUQ )
REGISTER_DAKOTA_ENUM( SUBMETHOD_QUESO,                    SUBMETHOD_QUESO)
REGISTER_DAKOTA_ENUM( SUBMETHOD_WASABI,                   SUBMETHOD_WASABI)
REGISTER_DAKOTA_ENUM( SUBMETHOD_CONMIN ,                  SUBMETHOD_CONMIN )
REGISTER_DAKOTA_ENUM( SUBMETHOD_DOT ,                     SUBMETHOD_DOT )
REGISTER_DAKOTA_ENUM( SUBMETHOD_NLPQL ,                   SUBMETHOD_NLPQL )
REGISTER_DAKOTA_ENUM( SUBMETHOD_NPSOL,                    SUBMETHOD_NPSOL)
REGISTER_DAKOTA_ENUM( SUBMETHOD_OPTPP ,                   SUBMETHOD_OPTPP )
REGISTER_DAKOTA_ENUM( SUBMETHOD_NPSOL_OPTPP,              SUBMETHOD_NPSOL_OPTPP)
REGISTER_DAKOTA_ENUM( SUBMETHOD_DIRECT ,                  SUBMETHOD_DIRECT )
REGISTER_DAKOTA_ENUM( SUBMETHOD_DIRECT_NPSOL_OPTPP,       SUBMETHOD_DIRECT_NPSOL_OPTPP)
REGISTER_DAKOTA_ENUM( SUBMETHOD_DIRECT_NPSOL ,            SUBMETHOD_DIRECT_NPSOL )
REGISTER_DAKOTA_ENUM( SUBMETHOD_DIRECT_OPTPP ,            SUBMETHOD_DIRECT_OPTPP )
REGISTER_DAKOTA_ENUM( SUBMETHOD_EA ,                      SUBMETHOD_EA )
REGISTER_DAKOTA_ENUM( SUBMETHOD_EGO ,                     SUBMETHOD_EGO )
REGISTER_DAKOTA_ENUM( SUBMETHOD_SBLO ,                    SUBMETHOD_SBLO )
REGISTER_DAKOTA_ENUM( SUBMETHOD_SBGO,                     SUBMETHOD_SBGO)
REGISTER_DAKOTA_ENUM( SUBMETHOD_AMV_X ,                   SUBMETHOD_AMV_X )
REGISTER_DAKOTA_ENUM( SUBMETHOD_AMV_U,                    SUBMETHOD_AMV_U)
REGISTER_DAKOTA_ENUM( SUBMETHOD_AMV_PLUS_X ,              SUBMETHOD_AMV_PLUS_X )
REGISTER_DAKOTA_ENUM( SUBMETHOD_AMV_PLUS_U,               SUBMETHOD_AMV_PLUS_U)
REGISTER_DAKOTA_ENUM( SUBMETHOD_TANA_X ,                  SUBMETHOD_TANA_X )
REGISTER_DAKOTA_ENUM( SUBMETHOD_TANA_U,                   SUBMETHOD_TANA_U)
REGISTER_DAKOTA_ENUM( SUBMETHOD_QMEA_X ,                  SUBMETHOD_QMEA_X )
REGISTER_DAKOTA_ENUM( SUBMETHOD_QMEA_U,                   SUBMETHOD_QMEA_U)
REGISTER_DAKOTA_ENUM( SUBMETHOD_NO_APPROX,                SUBMETHOD_NO_APPROX)
REGISTER_DAKOTA_ENUM( SUBMETHOD_EGRA_X ,                  SUBMETHOD_EGRA_X )
REGISTER_DAKOTA_ENUM( SUBMETHOD_EGRA_U,                   SUBMETHOD_EGRA_U)
REGISTER_DAKOTA_ENUM( SUBMETHOD_CONVERGE_ORDER ,          SUBMETHOD_CONVERGE_ORDER )
REGISTER_DAKOTA_ENUM( SUBMETHOD_CONVERGE_QOI,             SUBMETHOD_CONVERGE_QOI)
REGISTER_DAKOTA_ENUM( SUBMETHOD_ESTIMATE_ORDER,           SUBMETHOD_ESTIMATE_ORDER)

/// Sampling method for variance based decomposition (VBD)
enum { VBD_BINNED=0, VBD_PICK_AND_FREEZE };

/// Graph recursion options for generalized ACV
enum { NO_GRAPH_RECURSION=0, KL_GRAPH_RECURSION, PARTIAL_GRAPH_RECURSION,
       FULL_GRAPH_RECURSION };
/// Model selection options for generalized ACV
enum { NO_MODEL_SELECTION=0, ALL_MODEL_COMBINATIONS };

// define special values for outputLevel within
// Iterator/Model/Interface/Approximation
#define ADD_ENUMS X(SILENT_OUTPUT) X(QUIET_OUTPUT) X(NORMAL_OUTPUT) X(VERBOSE_OUTPUT) \
       X(DEBUG_OUTPUT)
#define X(name) name,
enum {
  ADD_ENUMS
};
#undef X
// Auto-register these enums when header is included
#define X(name) REGISTER_DAKOTA_ENUM(name, name)
  ADD_ENUMS
#undef X
#undef ADD_ENUMS

// define special values for printing of different results states
enum { NO_RESULTS=0,        // suppress all results
       REFINEMENT_RESULTS,  // results following a (minor) refinement iteration
       INTERMEDIATE_RESULTS,// results following a (major) alg stage/model level
       FINAL_RESULTS };     // final UQ results (throttled if subIterator)

// define special values for method synchronization (COLINY, APPS, EGO)
enum { DEFAULT_SYNCHRONIZATION=0, BLOCKING_SYNCHRONIZATION,
       NONBLOCKING_SYNCHRONIZATION };

// define special values for Iterator and Interface scheduling
enum { DEFAULT_SCHEDULING, DEDICATED_SCHEDULER_DYNAMIC, PEER_SCHEDULING,
       PEER_DYNAMIC_SCHEDULING, PEER_STATIC_SCHEDULING, DYNAMIC_SCHEDULING,
       STATIC_SCHEDULING };
// define special values for ParallelLibrary configuration logic
// related to scheduling
enum { DEFAULT_CONFIG, PUSH_DOWN, PUSH_UP };

// ----
// NonD
// ----
// define special values for u-space type used for random var transformations
enum { STD_NORMAL_U, STD_UNIFORM_U, PARTIAL_ASKEY_U, ASKEY_U, EXTENDED_U };
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
REGISTER_DAKOTA_ENUM( CUMULATIVE,    CUMULATIVE)
REGISTER_DAKOTA_ENUM( COMPLEMENTARY, COMPLEMENTARY)

// -------------
// NonDExpansion (most enums defined by Pecos in pecos_global_defs.hpp)
// -------------
// define special values for lsRegressionType
enum { DEFAULT_LS=0, SVD_LS, EQ_CON_LS };
// define special values for multilevAllocControl
enum { DEFAULT_MLMF_CONTROL=0, ESTIMATOR_VARIANCE, RIP_SAMPLING, RANK_SAMPLING,
       GREEDY_REFINEMENT };
// define special values for multilevDiscrepEmulation
enum { DEFAULT_EMULATION, DISTINCT_EMULATION, RECURSIVE_EMULATION };

// --------------------
// NonDBayesCalibration
// --------------------
// define special values for emulatorType
enum { NO_EMULATOR, PCE_EMULATOR, ML_PCE_EMULATOR, MF_PCE_EMULATOR, SC_EMULATOR,
       MF_SC_EMULATOR, GP_EMULATOR, KRIGING_EMULATOR, EXPGP_EMULATOR,
       VPS_EMULATOR };
// modes for calibrating multipliers on observational error
enum { CALIBRATE_NONE = 0, CALIBRATE_ONE, CALIBRATE_PER_EXPER,
       CALIBRATE_PER_RESP, CALIBRATE_BOTH};

// ------------
// NonDSampling
// ------------
// LHS rank array processing modes:
enum { IGNORE_RANKS, SET_RANKS, GET_RANKS, SET_GET_RANKS };
// sampling modes (combination of view and native distribution vs. uniform):
enum { DESIGN,            //DESIGN_UNIFORM,
       UNCERTAIN,           UNCERTAIN_UNIFORM,
       ALEATORY_UNCERTAIN,  ALEATORY_UNCERTAIN_UNIFORM,
       EPISTEMIC_UNCERTAIN, EPISTEMIC_UNCERTAIN_UNIFORM,
       STATE,             //STATE_UNIFORM,
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
REGISTER_DAKOTA_ENUM( ONE_SIDED_LOWER, ONE_SIDED_LOWER)
REGISTER_DAKOTA_ENUM( ONE_SIDED_UPPER, ONE_SIDED_UPPER)
REGISTER_DAKOTA_ENUM( TWO_SIDED,       TWO_SIDED)

// type of final statistics for NonD sampling methods
enum { DEFAULT_FINAL_STATS=0, QOI_STATISTICS, ESTIMATOR_PERFORMANCE };

// define special values for qoi aggregation norm for sample
// allocation over levels and QoIs
enum { QOI_AGGREGATION_MAX, QOI_AGGREGATION_SUM };
// target variance for fitting sample allocation
enum { TARGET_MEAN, TARGET_VARIANCE, TARGET_SIGMA, TARGET_SCALARIZATION };
// define special values for assessment of iterative refinement against a
// specified convergence tolerance, either as an absolute or relative difference
enum {  DEFAULT_CONVERGENCE_TOLERANCE=0, RELATIVE_CONVERGENCE_TOLERANCE,
       ABSOLUTE_CONVERGENCE_TOLERANCE };
// define optimization formulation for MLMC sample allocation by
// specifing equality constraint, either variance or cost
enum { VARIANCE_CONSTRAINT_TARGET, COST_CONSTRAINT_TARGET };

// Low disrepancy options
// rank 1 lattice options
// for the generating vector
enum { GEN_VECTOR_KUO, GEN_VECTOR_COOLS_KUO_NUYENS};
// ordering for rank 1 lattice approach
enum { RANK_1_LATTICE_NATURAL_ORDERING, RANK_1_LATTICE_RADICAL_INVERSE_ORDERING };

// digital net options
// ordering for digital net approach
enum { DIGITAL_NET_NATURAL_ORDERING, DIGITAL_NET_GRAY_CODE_ORDERING };
// generating matrix options
enum { JOE_KUO, SOBOL_ORDER_2 };

// ML/MF sampling modes
enum { ONLINE_PILOT,            OFFLINE_PILOT,
       ONLINE_PILOT_PROJECTION, OFFLINE_PILOT_PROJECTION };
// ML/MF modes for group-based pilot sampling
enum { SHARED_PILOT, INDEPENDENT_PILOT };
// Throttles for group-based pilot sampling
enum { NO_GROUP_THROTTLE=0,      MFMC_ESTIMATOR_GROUPS,
       COMMON_ESTIMATOR_GROUPS,  GROUP_SIZE_THROTTLE,
       RCOND_TOLERANCE_THROTTLE, RCOND_BEST_COUNT_THROTTLE };
// special values for optSubProblemForm
enum { NO_OPTIMAL_ALLOCATION=0, ANALYTIC_SOLUTION, REORDERED_ANALYTIC_SOLUTION,
       R_ONLY_LINEAR_CONSTRAINT, R_AND_N_NONLINEAR_CONSTRAINT,
       N_MODEL_LINEAR_CONSTRAINT, N_MODEL_LINEAR_OBJECTIVE,
       N_GROUP_LINEAR_CONSTRAINT, N_GROUP_LINEAR_OBJECTIVE };
// Numerical solution modes
enum { REORDERED_FALLBACK, NUMERICAL_FALLBACK, NUMERICAL_OVERRIDE };
// Model reordering modes during numerical solutions
enum { FIXED_MODEL_ORDERING, REORDER_MODELS_ON_THE_FLY };
// options for obtaining cost data for model fidelities/resolutions
enum { NO_COST_SOURCE=0, USER_COST_SPEC, ONLINE_COST_RECOVERY,
       MIXED_COST_SPEC_RECOVERY };
// options for metrics derived from QoI estimator variances
enum { DEFAULT_ESTVAR_METRIC = 0, AVG_ESTVAR_METRIC, NORM_ESTVAR_METRIC,
       MAX_ESTVAR_METRIC, AVG_ESTVAR_RATIO_METRIC, NORM_ESTVAR_RATIO_METRIC,
       MAX_ESTVAR_RATIO_METRIC };

// ---------------
// NonDReliability
// ---------------
// define special values for mppSearchType
//enum { MV=0, AMV_X, AMV_U, AMV_PLUS_X, AMV_PLUS_U, TANA_X, TANA_U,
//       QMEA_X, QMEA_U, NO_APPROX, EGRA_X, EGRA_U };
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

// This is needed to map pecos enums to strings for json input parsing - RWH
REGISTER_DAKOTA_ENUM( NO_MOMENTS,       Pecos::NO_MOMENTS)
REGISTER_DAKOTA_ENUM( STANDARD_MOMENTS, Pecos::STANDARD_MOMENTS)
REGISTER_DAKOTA_ENUM( CENTRAL_MOMENTS,  Pecos::CENTRAL_MOMENTS)


} // namespace Dakota
