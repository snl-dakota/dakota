#pragma once

namespace Dakota {

/// define special values for pointsManagement
enum { DEFAULT_POINTS, MINIMUM_POINTS, RECOMMENDED_POINTS, TOTAL_POINTS };

/// define special values for SurrogateModel::responseMode
enum { DEFAULT_SURROGATE_RESP_MODE=0, NO_SURROGATE, UNCORRECTED_SURROGATE,
       AUTO_CORRECTED_SURROGATE, BYPASS_SURROGATE, MODEL_DISCREPANCY,
       AGGREGATED_MODEL_PAIR, AGGREGATED_MODELS };

/// define special values for approxCorrectionType
enum { NO_CORRECTION=0,  ADDITIVE_CORRECTION, MULTIPLICATIVE_CORRECTION,
       COMBINED_CORRECTION };

/// define types of random field approximations
enum { RF_KARHUNEN_LOEVE=0, RF_PCA_GP, RF_ICA };

/// define types of analytic covariance functions
enum { NOCOVAR=0, EXP_L2, EXP_L1 };

/// define special values for active subspace normalizations
enum { SUBSPACE_NORM_DEFAULT=0, SUBSPACE_NORM_MEAN_VALUE,
       SUBSPACE_NORM_MEAN_GRAD, SUBSPACE_NORM_LOCAL_GRAD }; 

/// define special values for componentParallelMode
/// (active model for parallel scheduling)
enum { NO_PARALLEL_MODE=0, SURROGATE_MODEL_MODE, TRUTH_MODEL_MODE,
       SUB_MODEL_MODE, INTERFACE_MODE };

/// define special values for distParamDerivs
enum { NO_DERIVS=0, ALL_DERIVS, MIXED_DERIVS }; 

/// define special values for mlmfPrecedence
enum { DEFAULT_PRECEDENCE=0, MULTILEVEL_PRECEDENCE, MULTIFIDELITY_PRECEDENCE,
       MULTILEVEL_MULTIFIDELITY_PRECEDENCE, ENUMERATION_PRECEDENCE };

// define special values for regressionType in C3 FT (outside of Pecos).
// Note that C3 and Pecos are mutually exclusive: use of values from multiple
// enums should not conflict
enum { FT_LS, FT_RLS2 };//, FT_RLSD2, FT_RLSRKHS, FT_RLS1 };
// define special values for c3AdvanceType
enum { NO_C3_ADVANCEMENT=0, START_RANK_ADVANCEMENT, START_ORDER_ADVANCEMENT,
       MAX_RANK_ADVANCEMENT, MAX_ORDER_ADVANCEMENT, MAX_RANK_ORDER_ADVANCEMENT};

// -----------------------
// AdaptedBasis
// -----------------------
// define special values for generating the basis adaptation rotation matrix
enum { ROTATION_METHOD_UNRANKED, ROTATION_METHOD_RANKED };    



} // namespace Dakota
