"""Generated Pydantic models for shared.expansion/pce"""

from __future__ import annotations

from ....base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ....base import SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ....validation import ValidationRule
from ....validation.rules import CheckNonnegativeList, CompareLength, CompareLengthOne


class PceCubatureMixin(DakotaBaseModel):
    "Generated model for PceCubatureMixin"

    cubature_integrand: int = DakotaField(
        description="Cubature using Stroud rules and their extensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cubature_integrand",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionImportMixin(DakotaBaseModel):
    "Generated model for PceExpansionImportMixin"

    import_expansion_file: str = DakotaField(
        description="Build a Polynomial Chaos Expansion (PCE) by importing expansion coefficients and a corresponding multi-index from a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.import_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MfPcePRefinementUniform(DakotaBaseModel):
    "Refine an expansion uniformly in all dimensions."

    uniform: Literal[True] = DakotaField(
        default=True,
        description="Refine an expansion uniformly in all dimensions.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNIFORM_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPcePRefinementDimAdaptiveSobol(DakotaBaseModel):
    "Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices."

    sobol: Literal[True] = DakotaField(
        default=True,
        description="Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_SOBOL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPcePRefinementDimAdaptiveDecay(DakotaBaseModel):
    "Estimate spectral coefficient decay rates to guide dimension-adaptive refinement."

    decay: Literal[True] = DakotaField(
        default=True,
        description="Estimate spectral coefficient decay rates to guide dimension-adaptive refinement.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_DECAY",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPcePRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    "Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion."

    generalized: Literal[True] = DakotaField(
        default=True,
        description="Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_GENERALIZED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceAllocationControlGreedy(DakotaBaseModel):
    "Sample allocation based on greedy refinement within multifidelity polynomial chaos"

    greedy: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on greedy refinement within multifidelity polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GREEDY_REFINEMENT",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceDiscrepancyEmulationDistinct(DakotaBaseModel):
    "Distinct formulation for emulation of model discrepancies."

    distinct: Literal[True] = DakotaField(
        default=True,
        description="Distinct formulation for emulation of model discrepancies.",
        dakota={
            "aliases": ["paired"],
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_discrepancy_emulation",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DISTINCT_EMULATION",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceDiscrepancyEmulationRecursive(DakotaBaseModel):
    "Recursive formulation for emulation of model discrepancies."

    recursive: Literal[True] = DakotaField(
        default=True,
        description="Recursive formulation for emulation of model discrepancies.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_discrepancy_emulation",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RECURSIVE_EMULATION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceQuadratureOrderSequenceNested(DakotaBaseModel):
    "Enforce use of nested quadrature rules if available"

    nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of nested quadrature rules if available",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceQuadratureOrderSequenceNonNested(DakotaBaseModel):
    "Enforce use of non-nested quadrature rules"

    non_nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of non-nested quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NON_NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceSGLevelSequenceRestricted(DakotaBaseModel):
    "Restrict the growth rates for nested and non-nested rules can be synchronized for consistency."

    restricted: Literal[True] = DakotaField(
        default=True,
        description="Restrict the growth rates for nested and non-nested rules can be synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceSGLevelSequenceUnrestricted(DakotaBaseModel):
    "Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency."

    unrestricted: Literal[True] = DakotaField(
        default=True,
        description="Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNRESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceExpansionSamplesSequenceConfig(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfpceexpansionordersequenceexpansionsamplessequenceconfig",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceAskey(DakotaBaseModel):
    "Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables."

    askey: Literal[True] = DakotaField(
        default=True,
        description="Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ASKEY_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceWiener(DakotaBaseModel):
    "Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space."

    wiener: Literal[True] = DakotaField(
        default=True,
        description="Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STD_NORMAL_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceDiagCov(DakotaBaseModel):
    "Display only the diagonal terms of the covariance matrix"

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display only the diagonal terms of the covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIAGONAL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceFullCov(DakotaBaseModel):
    "Display the full covariance matrix"

    full_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display the full covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FULL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceAllocationControlEstimatorVarianceConfig(DakotaBaseModel):
    "Variance of mean estimator within multilevel polynomial chaos"

    estimator_rate: DakotaFloat = DakotaField(
        default=2.0,
        description="Rate of convergence of mean estimator within multilevel polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_estimator_rate",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class BayesMlPceAllocationControlRipSampling(DakotaBaseModel):
    "Sample allocation based on restricted isometry property (RIP) within multilevel polynomial chaos"

    rip_sampling: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on restricted isometry property (RIP) within multilevel polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RIP_SAMPLING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceDiscrepancyEmulationDistinct(DakotaBaseModel):
    "Distinct formulation for emulation of model discrepancies."

    distinct: Literal[True] = DakotaField(
        default=True,
        description="Distinct formulation for emulation of model discrepancies.",
        dakota={
            "aliases": ["paired"],
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_discrepancy_emulation",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DISTINCT_EMULATION",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceDiscrepancyEmulationRecursive(DakotaBaseModel):
    "Recursive formulation for emulation of model discrepancies."

    recursive: Literal[True] = DakotaField(
        default=True,
        description="Recursive formulation for emulation of model discrepancies.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_discrepancy_emulation",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RECURSIVE_EMULATION",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmlpceexpansionordersequenceexpansionsamplessequenceconfig",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MlPceAskey(DakotaBaseModel):
    "Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables."

    askey: Literal[True] = DakotaField(
        default=True,
        description="Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ASKEY_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceWiener(DakotaBaseModel):
    "Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space."

    wiener: Literal[True] = DakotaField(
        default=True,
        description="Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STD_NORMAL_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceDiagCov(DakotaBaseModel):
    "Display only the diagonal terms of the covariance matrix"

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display only the diagonal terms of the covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIAGONAL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MlPceFullCov(DakotaBaseModel):
    "Display the full covariance matrix"

    full_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display the full covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FULL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPcePRefinementUniform(DakotaBaseModel):
    "Refine an expansion uniformly in all dimensions."

    uniform: Literal[True] = DakotaField(
        default=True,
        description="Refine an expansion uniformly in all dimensions.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNIFORM_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPcePRefinementDimAdaptiveSobol(DakotaBaseModel):
    "Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices."

    sobol: Literal[True] = DakotaField(
        default=True,
        description="Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_SOBOL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPcePRefinementDimAdaptiveDecay(DakotaBaseModel):
    "Estimate spectral coefficient decay rates to guide dimension-adaptive refinement."

    decay: Literal[True] = DakotaField(
        default=True,
        description="Estimate spectral coefficient decay rates to guide dimension-adaptive refinement.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_DECAY",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPcePRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    "Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion."

    generalized: Literal[True] = DakotaField(
        default=True,
        description="Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_GENERALIZED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceQuadratureOrderNested(DakotaBaseModel):
    "Enforce use of nested quadrature rules if available"

    nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of nested quadrature rules if available",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceQuadratureOrderNonNested(DakotaBaseModel):
    "Enforce use of non-nested quadrature rules"

    non_nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of non-nested quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NON_NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceSGLevelRestricted(DakotaBaseModel):
    "Restrict the growth rates for nested and non-nested rules can be synchronized for consistency."

    restricted: Literal[True] = DakotaField(
        default=True,
        description="Restrict the growth rates for nested and non-nested rules can be synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceSGLevelUnrestricted(DakotaBaseModel):
    "Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency."

    unrestricted: Literal[True] = DakotaField(
        default=True,
        description="Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNRESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceCubatureIntegrand(DakotaBaseModel):
    "Cubature using Stroud rules and their extensions"

    cubature_integrand: int = DakotaField(
        description="Cubature using Stroud rules and their extensions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cubature_integrand",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderExpansionSamplesConfig(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    value: int = DakotaField(
        default=SZ_MAX,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceImportExpansionFile(DakotaBaseModel):
    "Build a Polynomial Chaos Expansion (PCE) by importing expansion coefficients and a corresponding multi-index from a file"

    import_expansion_file: str = DakotaField(
        description="Build a Polynomial Chaos Expansion (PCE) by importing expansion coefficients and a corresponding multi-index from a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.import_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BayesPceAskey(DakotaBaseModel):
    "Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables."

    askey: Literal[True] = DakotaField(
        default=True,
        description="Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ASKEY_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceWiener(DakotaBaseModel):
    "Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space."

    wiener: Literal[True] = DakotaField(
        default=True,
        description="Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STD_NORMAL_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceDiagCov(DakotaBaseModel):
    "Display only the diagonal terms of the covariance matrix"

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display only the diagonal terms of the covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIAGONAL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceFullCov(DakotaBaseModel):
    "Display the full covariance matrix"

    full_covariance: Literal[True] = DakotaField(
        default=True,
        description="Display the full covariance matrix",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.covariance_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "FULL_COVARIANCE",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeTensorProductVariant1(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeTotalOrderVariant1(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeAdaptedConfigVariant1(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsLeastSquaresSvdVariant1(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsLeastSquaresEqConVariant1(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsOMPConfigVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsBPVariant2(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsLassoConfigVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsCVVariant1(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderExpansionSamplesConfigVariant1(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    value: int = DakotaField(
        default=SZ_MAX,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfigVariant1(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileAnnotatedVariant1(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileFreeformVariant1(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPointsLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPointsLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPointsOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPointsBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocPtsWithPceRegressCollocPointsLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPointsCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioWithPceRegressCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocPointsBPDNConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsLarsConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocRatioBPDNConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioLarsConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceExpansionOrderExpansionSamplesConfig(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    value: int = DakotaField(
        default=SZ_MAX,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceOrthogLeastInterpImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class CorePceOrthogLeastInterpImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MlpceAllocControlAllocationControlEstimatorVarianceConfig(DakotaBaseModel):
    "Variance of mean estimator within multilevel polynomial chaos"

    estimator_rate: DakotaFloat = DakotaField(
        default=2.0,
        description="Rate of convergence of mean estimator within multilevel polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_estimator_rate",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class MlpceAllocControlAllocationControlRipSampling(DakotaBaseModel):
    "Sample allocation based on restricted isometry property (RIP) within multilevel polynomial chaos"

    rip_sampling: Literal[True] = DakotaField(
        default=True,
        description="Sample allocation based on restricted isometry property (RIP) within multilevel polynomial chaos",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.multilevel_allocation_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RIP_SAMPLING",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderCollocPointsLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderExpansionSamplesConfig(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    value: int = DakotaField(
        default=SZ_MAX,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class PceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceBasisTypeTensorProduct(DakotaBaseModel):
    "Use a tensor-product index set to construct a polynomial chaos expansion."

    tensor_product: Literal[True] = DakotaField(
        default=True,
        description="Use a tensor-product index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceBasisTypeTotalOrder(DakotaBaseModel):
    "Use a total-order index set to construct a polynomial chaos expansion."

    total_order: Literal[True] = DakotaField(
        default=True,
        description="Use a total-order index set to construct a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceBasisTypeAdaptedConfig(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    advancements: int = DakotaField(
        default=3,
        description="The maximum number of steps used to expand a basis step.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adapted_basis.advancements",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    soft_convergence_limit: int = DakotaField(
        default=0,
        description="The maximum number of times no improvement in cross validation error is allowed before the algorithm is terminated.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.soft_convergence_limit",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderSequenceCollocRatioLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceExpansionSamplesSequenceConfig(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceexpansionordersequenceexpansionsamplessequenceconfig",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceImportBuildPointsFileAnnotated(DakotaBaseModel):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceImportBuildPointsFileFreeform(DakotaBaseModel):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceExpansionSamplesExpansionSamples(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    value: int = DakotaField(
        default=SZ_MAX,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class PceExpansionSamplesSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceexpansionsamplessequenceexpansionsamplessequence",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_samples_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class PceOptionsAskey(DakotaBaseModel):
    "Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables."

    askey: Literal[True] = DakotaField(
        default=True,
        description="Select the standardized random variables (and associated basis polynomials) from the Askey family that best match the user-specified random variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ASKEY_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceOptionsWiener(DakotaBaseModel):
    "Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space."

    wiener: Literal[True] = DakotaField(
        default=True,
        description="Use standard normal random variables (along with Hermite orthogonal basis polynomials) when transforming to a standardized probability space.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STD_NORMAL_U",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_HEADER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    eval_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable evaluation ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_EVAL_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    interface_id: Literal[True] | None = DakotaField(
        default=None,
        description="Enable interface ID column in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "AUGMENT_ENUM",
                    "stored_value": "TABULAR_IFACE_ID",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileAnnotated(
    DakotaBaseModel
):
    "Selects annotated tabular file format"

    annotated: Literal[True] = DakotaField(
        default=True,
        description="Selects annotated tabular file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_ANNOTATED",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileFreeform(
    DakotaBaseModel
):
    "Selects freeform file format"

    freeform: Literal[True] = DakotaField(
        default=True,
        description="Selects freeform file format",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceRefinementPRefinementUniform(DakotaBaseModel):
    "Refine an expansion uniformly in all dimensions."

    uniform: Literal[True] = DakotaField(
        default=True,
        description="Refine an expansion uniformly in all dimensions.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNIFORM_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRefinementPRefinementDimAdaptiveSobol(DakotaBaseModel):
    "Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices."

    sobol: Literal[True] = DakotaField(
        default=True,
        description="Estimate dimension preference for automated refinement of stochastic expansion using total Sobol' sensitivity indices.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_SOBOL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRefinementPRefinementDimAdaptiveDecay(DakotaBaseModel):
    "Estimate spectral coefficient decay rates to guide dimension-adaptive refinement."

    decay: Literal[True] = DakotaField(
        default=True,
        description="Estimate spectral coefficient decay rates to guide dimension-adaptive refinement.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_DECAY",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRefinementPRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    "Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion."

    generalized: Literal[True] = DakotaField(
        default=True,
        description="Use the generalized sparse grid dimension adaptive algorithm to refine a sparse grid approximation of stochastic expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DIMENSION_ADAPTIVE_CONTROL_GENERALIZED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRegressionAlgSelectLeastSquaresSvd(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition."

    svd: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using the singular value decomposition.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SVD_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRegressionAlgSelectLeastSquaresEqCon(DakotaBaseModel):
    "Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares."

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        description="Calculate the coefficients of a polynomial chaos expansion using equality constrained least squares.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "EQ_CON_LS",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRegressionAlgSelectOMPConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class PceRegressionAlgSelectBP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming."

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit L1 -minimization problem using linear programming.",
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceRegressionAlgSelectBPDNConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class PceRegressionAlgSelectLarsConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class PceRegressionAlgSelectLassoConfig(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    noise_tolerance: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="The noise tolerance used when performing cross validation in the presence of noise or truncation errors.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_noise_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    l2_penalty: DakotaFloat | None = DakotaField(
        default=None,
        description="The :math:`l_2`  pentalty used when performing compressed sensing with elastic net.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class PceRegressionOptsCV(DakotaBaseModel):
    "Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion."

    noise_only: Literal[True] | None = DakotaField(
        default=None,
        description="Restrict the cross validation process to estimating only the best noise tolerance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.noise_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    max_cv_order_candidates: int = DakotaField(
        default=USHRT_MAX,
        ge=0,
        description="Limit the number of cross-validation candidates for basis order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation.max_order_candidates",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class PceSGLevelRestricted(DakotaBaseModel):
    "Restrict the growth rates for nested and non-nested rules can be synchronized for consistency."

    restricted: Literal[True] = DakotaField(
        default=True,
        description="Restrict the growth rates for nested and non-nested rules can be synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGLevelUnrestricted(DakotaBaseModel):
    "Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency."

    unrestricted: Literal[True] = DakotaField(
        default=True,
        description="Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNRESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGLevelNested(DakotaBaseModel):
    "Enforce use of nested quadrature rules if available"

    nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of nested quadrature rules if available",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGLevelNonNested(DakotaBaseModel):
    "Enforce use of non-nested quadrature rules"

    non_nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of non-nested quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NON_NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGSequenceLevelSequenceRestricted(DakotaBaseModel):
    "Restrict the growth rates for nested and non-nested rules can be synchronized for consistency."

    restricted: Literal[True] = DakotaField(
        default=True,
        description="Restrict the growth rates for nested and non-nested rules can be synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGSequenceLevelSequenceUnrestricted(DakotaBaseModel):
    "Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency."

    unrestricted: Literal[True] = DakotaField(
        default=True,
        description="Overide the default restriction of growth rates for nested and non-nested rules that are by defualt synchronized for consistency.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "UNRESTRICTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGSequenceLevelSequenceNested(DakotaBaseModel):
    "Enforce use of nested quadrature rules if available"

    nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of nested quadrature rules if available",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceSGSequenceLevelSequenceNonNested(DakotaBaseModel):
    "Enforce use of non-nested quadrature rules"

    non_nested: Literal[True] = DakotaField(
        default=True,
        description="Enforce use of non-nested quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NON_NESTED",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPcePRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        MfPcePRefinementDimAdaptiveSobol,
        MfPcePRefinementDimAdaptiveDecay,
        MfPcePRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class BayesMfPceQuadratureOrderSequenceConfig(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfpcequadratureordersequenceconfig", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.quadrature_order_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    nesting_rule: (
        Union[
            BayesMfPceQuadratureOrderSequenceNested,
            BayesMfPceQuadratureOrderSequenceNonNested,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfPceSGLevelSequenceConfig(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfpcesglevelsequenceconfig", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.sparse_grid_level_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    growth_rule: (
        Union[
            BayesMfPceSGLevelSequenceRestricted, BayesMfPceSGLevelSequenceUnrestricted
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[
            BayesMfPceQuadratureOrderSequenceNested,
            BayesMfPceQuadratureOrderSequenceNonNested,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfPceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: BayesMfPceExpansionOrderSequenceBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: BayesMfPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: BayesMfPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: BayesMfPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequenceCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: BayesMfPceExpansionOrderSequenceCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    expansion_samples_sequence: BayesMfPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesMfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesMlPceAllocationControlEstimatorVariance(DakotaBaseModel):
    "Variance of mean estimator within multilevel polynomial chaos"

    estimator_variance: BayesMlPceAllocationControlEstimatorVarianceConfig = (
        DakotaField(
            description="Variance of mean estimator within multilevel polynomial chaos",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.multilevel_allocation_control",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "ESTIMATOR_VARIANCE",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class BayesMlPceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: BayesMlPceExpansionOrderSequenceBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: BayesMlPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: BayesMlPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: BayesMlPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: BayesMlPceExpansionOrderSequenceCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    expansion_samples_sequence: BayesMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesMlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesPcePRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        BayesPcePRefinementDimAdaptiveSobol,
        BayesPcePRefinementDimAdaptiveDecay,
        BayesPcePRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class BayesPceQuadratureOrderConfig(DakotaBaseModel):
    "Order for tensor-products of Gaussian quadrature rules"

    order: int = DakotaField(
        default=USHRT_MAX,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.quadrature_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    nesting_rule: (
        Union[BayesPceQuadratureOrderNested, BayesPceQuadratureOrderNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesPceSGLevelConfig(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    level: int = DakotaField(
        default=USHRT_MAX,
        description="Level to use in sparse grid integration or interpolation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.sparse_grid_level",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    growth_rule: (
        Union[BayesPceSGLevelRestricted, BayesPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[BayesPceQuadratureOrderNested, BayesPceQuadratureOrderNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesPceExpansionOrderBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: BayesPceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderBasisTypeAdaptedVariant2(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: BayesPceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsLeastSquaresVariant3(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocRatioLeastSquaresVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsOMPVariant2(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsBPDNVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsLarsVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocRatioOMPVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: BayesPceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: BayesPceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: BayesPceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocPointsLassoVariant2(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: BayesPceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderCollocRatioLassoVariant1(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: BayesPceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderExpansionSamples(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    expansion_samples: BayesPceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class BayesPceExpansionOrderExpansionSamplesVariant2(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    expansion_samples: BayesPceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class BayesPceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedVariant2(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class CollocPtsWithPceRegressCollocPointsLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        CollocPtsWithPceRegressCollocPointsLeastSquaresSvd,
        CollocPtsWithPceRegressCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPointsOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: CollocPtsWithPceRegressCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocPtsWithPceRegressCollocPointsBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: CollocPtsWithPceRegressCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocPtsWithPceRegressCollocPointsLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: CollocPtsWithPceRegressCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocPtsWithPceRegressCollocPointsLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: CollocPtsWithPceRegressCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresSvd,
        CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: CollocRatioPtsSequenceWithPceRegressCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: CollocRatioPtsSequenceWithPceRegressCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: CollocRatioPtsSequenceWithPceRegressCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: CollocRatioPtsSequenceWithPceRegressCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioWithPceRegressCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        CollocRatioWithPceRegressCollocRatioLeastSquaresSvd,
        CollocRatioWithPceRegressCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: CollocRatioWithPceRegressCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioWithPceRegressCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: CollocRatioWithPceRegressCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioWithPceRegressCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: CollocRatioWithPceRegressCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CollocRatioWithPceRegressCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: CollocRatioWithPceRegressCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: CorePceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: CorePceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=CorePceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "CorePceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocPointsLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        CorePceExpansionOrderCollocPointsLeastSquaresSvd,
        CorePceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: CorePceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocPointsBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: CorePceExpansionOrderCollocPointsBPDNConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocPointsLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: CorePceExpansionOrderCollocPointsLarsConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocPointsLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: CorePceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        CorePceExpansionOrderCollocRatioLeastSquaresSvd,
        CorePceExpansionOrderCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: CorePceExpansionOrderCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: CorePceExpansionOrderCollocRatioBPDNConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: CorePceExpansionOrderCollocRatioLarsConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: CorePceExpansionOrderCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderExpansionSamples(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    expansion_samples: CorePceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MlpceAllocControlAllocationControlEstimatorVariance(DakotaBaseModel):
    "Variance of mean estimator within multilevel polynomial chaos"

    estimator_variance: MlpceAllocControlAllocationControlEstimatorVarianceConfig = (
        DakotaField(
            description="Variance of mean estimator within multilevel polynomial chaos",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.multilevel_allocation_control",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "ESTIMATOR_VARIANCE",
                        "ir_value_type": "short",
                    }
                ]
            },
        )
    )


class PceBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: PceBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: PceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        PceExpansionOrderCollocPointsLeastSquaresSvd,
        PceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        PceExpansionOrderCollocPointsLeastSquaresSvd,
        PceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: PceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderCollocPointsBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: PceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderCollocPointsLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: PceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: PceExpansionOrderCollocPointsOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderCollocPointsLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: PceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: PceExpansionOrderCollocPointsLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderExpansionSamples(DakotaBaseModel):
    "Number of simulation samples used to estimate the expected value of a set of PCE coefficients"

    expansion_samples: PceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class PceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: PceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=PceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "PceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class PceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    "Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion."

    adapted: PceExpansionOrderSequenceBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        PceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        PceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: PceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderSequenceCollocRatioBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: PceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderSequenceCollocRatioLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: PceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderSequenceCollocRatioLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: PceExpansionOrderSequenceCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    "Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`"

    expansion_samples_sequence: PceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    "Selects custom-annotated tabular file format"

    custom_annotated: PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class PceExpansionSamplesMixin(DakotaBaseModel):
    "Generated model for PceExpansionSamplesMixin"

    expansion_samples: PceExpansionSamplesExpansionSamples = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class PceExpansionSamplesSequenceMixin(DakotaBaseModel):
    "Generated model for PceExpansionSamplesSequenceMixin"

    expansion_samples_sequence: PceExpansionSamplesSequenceExpansionSamplesSequence = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class PceOptionsMixin(DakotaBaseModel):
    "Generated model for PceOptionsMixin"

    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    basis_family: Union[PceOptionsAskey, PceOptionsWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )


class PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    "Selects custom-annotated tabular file format"

    custom_annotated: PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "TABULAR_NONE",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class PceRefinementPRefinementDimAdaptive(DakotaBaseModel):
    'Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher \\"importance\\".'

    dimension_adaptive: Union[
        PceRefinementPRefinementDimAdaptiveSobol,
        PceRefinementPRefinementDimAdaptiveDecay,
        PceRefinementPRefinementDimAdaptiveGeneralized,
    ] = DakotaField(
        description='Perform anisotropic expansion refinement by preferentially adapting in dimensions that are detected to have higher "importance".'
    )


class PceRegressionAlgSelectLeastSquares(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using least squares"

    least_squares: Union[
        PceRegressionAlgSelectLeastSquaresSvd,
        PceRegressionAlgSelectLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        description="Compute the coefficients of a polynomial expansion using least squares",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class PceRegressionAlgSelectOMP(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)"

    orthogonal_matching_pursuit: PceRegressionAlgSelectOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceRegressionAlgSelectBPDN(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization."

    basis_pursuit_denoising: PceRegressionAlgSelectBPDNConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceRegressionAlgSelectLars(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method."

    least_angle_regression: PceRegressionAlgSelectLarsConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceRegressionAlgSelectLasso(DakotaBaseModel):
    "Compute the coefficients of a polynomial expansion by using the LASSO problem."

    least_absolute_shrinkage: PceRegressionAlgSelectLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LASSO_REGRESSION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceRegressionOptsMixin(DakotaBaseModel):
    "Generated model for PceRegressionOptsMixin"

    cross_validation: PceRegressionOptsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class PceSGSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    level: int = DakotaField(
        default=USHRT_MAX,
        description="Level to use in sparse grid integration or interpolation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.sparse_grid_level",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    growth_rule: Union[PceSGLevelRestricted, PceSGLevelUnrestricted] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Growth",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )
    nesting_rule: Union[PceSGLevelNested, PceSGLevelNonNested] | None = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class PceSGSequenceSGLevelSequence(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pcesgsequencesglevelsequence", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.sparse_grid_level_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    growth_rule: (
        Union[
            PceSGSequenceLevelSequenceRestricted, PceSGSequenceLevelSequenceUnrestricted
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[PceSGSequenceLevelSequenceNested, PceSGSequenceLevelSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfPceQuadratureOrderSequence(DakotaBaseModel):
    "Sequence of quadrature orders used in a multi-stage expansion"

    quadrature_order_sequence: BayesMfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfPceSGLevelSequence(DakotaBaseModel):
    "Sequence of sparse grid levels used in a multi-stage expansion"

    sparse_grid_level_sequence: BayesMfPceSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfpceexpansionordersequencecollocratioconfig",
            list_field="collocation_points_sequence",
        ),
    ]

    sequence: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    collocation_points_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "aliases": ["pilot_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )
    regression_method: (
        Union[
            BayesMfPceExpansionOrderSequenceCollocRatioLeastSquares,
            BayesMfPceExpansionOrderSequenceCollocRatioOMP,
            BayesMfPceExpansionOrderSequenceCollocRatioBP,
            BayesMfPceExpansionOrderSequenceCollocRatioBPDN,
            BayesMfPceExpansionOrderSequenceCollocRatioLars,
            BayesMfPceExpansionOrderSequenceCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: BayesMfPceExpansionOrderSequenceCollocRatioCV | None = (
        DakotaField(
            default=None,
            description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.cross_validation",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BayesMfPceExpansionOrderSequenceImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated,
        BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        BayesMfPceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesMfPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesMfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        BayesMfPceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmlpceexpansionordersequencecollocratioconfig",
            list_field="collocation_points_sequence",
        ),
    ]

    sequence: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    collocation_points_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "aliases": ["pilot_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )
    regression_method: (
        Union[
            BayesMlPceExpansionOrderSequenceCollocRatioLeastSquares,
            BayesMlPceExpansionOrderSequenceCollocRatioOMP,
            BayesMlPceExpansionOrderSequenceCollocRatioBP,
            BayesMlPceExpansionOrderSequenceCollocRatioBPDN,
            BayesMlPceExpansionOrderSequenceCollocRatioLars,
            BayesMlPceExpansionOrderSequenceCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: BayesMlPceExpansionOrderSequenceCollocRatioCV | None = (
        DakotaField(
            default=None,
            description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.cross_validation",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BayesMlPceExpansionOrderSequenceImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated,
        BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        BayesMlPceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesMlPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesMlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        BayesMlPceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceQuadratureOrder(DakotaBaseModel):
    "Order for tensor-products of Gaussian quadrature rules"

    quadrature_order: BayesPceQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class BayesPceSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

    sparse_grid_level: BayesPceSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class BayesPceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    regression_method: (
        Union[
            BayesPceExpansionOrderCollocPointsLeastSquares,
            BayesPceExpansionOrderCollocPointsOMP,
            BayesPceExpansionOrderCollocPointsBP,
            BayesPceExpansionOrderCollocPointsBPDN,
            BayesPceExpansionOrderCollocPointsLars,
            BayesPceExpansionOrderCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: BayesPceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocPointsConfigVariant1(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    regression_method: (
        Union[
            BayesPceExpansionOrderCollocPointsLeastSquares,
            BayesPceExpansionOrderCollocPointsOMP,
            BayesPceExpansionOrderCollocPointsBP,
            BayesPceExpansionOrderCollocPointsBPDN,
            BayesPceExpansionOrderCollocPointsLars,
            BayesPceExpansionOrderCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: BayesPceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    value: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    regression_method: (
        Union[
            BayesPceExpansionOrderCollocRatioLeastSquares,
            BayesPceExpansionOrderCollocRatioOMP,
            BayesPceExpansionOrderCollocPointsBP,
            BayesPceExpansionOrderCollocPointsBPDN,
            BayesPceExpansionOrderCollocPointsLars,
            BayesPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: BayesPceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BayesPceExpansionOrderCollocRatioConfigVariant1(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    value: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    regression_method: (
        Union[
            BayesPceExpansionOrderCollocRatioLeastSquares,
            BayesPceExpansionOrderCollocRatioOMP,
            BayesPceExpansionOrderCollocPointsBP,
            BayesPceExpansionOrderCollocPointsBPDN,
            BayesPceExpansionOrderCollocPointsLars,
            BayesPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: BayesPceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BayesPceExpansionOrderImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesPceExpansionOrderImportBuildPointsFileCustomAnnotated,
        BayesPceExpansionOrderImportBuildPointsFileAnnotated,
        BayesPceExpansionOrderImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesPceExpansionOrderImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesPceExpansionOrderImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceExpansionOrderImportBuildPointsFileVariant5(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesPceExpansionOrderImportBuildPointsFileCustomAnnotated,
        BayesPceExpansionOrderImportBuildPointsFileAnnotated,
        BayesPceExpansionOrderImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesPceExpansionOrderImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesPceExpansionOrderImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        BayesPceExpansionOrderImportBuildPointsFileAnnotated,
        BayesPceExpansionOrderImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesPceExpansionOrderImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesPceExpansionOrderImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated,
        BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class CollocPtsWithPceRegressCollocPoints(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    regression_method: (
        Union[
            CollocPtsWithPceRegressCollocPointsLeastSquares,
            CollocPtsWithPceRegressCollocPointsOMP,
            CollocPtsWithPceRegressCollocPointsBP,
            CollocPtsWithPceRegressCollocPointsBPDN,
            CollocPtsWithPceRegressCollocPointsLars,
            CollocPtsWithPceRegressCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: CollocPtsWithPceRegressCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="collocratioptssequencewithpceregresscollocratio",
            list_field="collocation_points_sequence",
        ),
    ]

    sequence: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    collocation_points_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "aliases": ["pilot_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )
    regression_method: (
        Union[
            CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquares,
            CollocRatioPtsSequenceWithPceRegressCollocRatioOMP,
            CollocRatioPtsSequenceWithPceRegressCollocRatioBP,
            CollocRatioPtsSequenceWithPceRegressCollocRatioBPDN,
            CollocRatioPtsSequenceWithPceRegressCollocRatioLars,
            CollocRatioPtsSequenceWithPceRegressCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: CollocRatioPtsSequenceWithPceRegressCollocRatioCV | None = (
        DakotaField(
            default=None,
            description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.cross_validation",
                        "storage_type": "PRESENCE_TRUE",
                        "ir_value_type": "bool",
                    }
                ]
            },
        )
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class CollocRatioWithPceRegressCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    value: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    regression_method: (
        Union[
            CollocRatioWithPceRegressCollocRatioLeastSquares,
            CollocRatioWithPceRegressCollocRatioOMP,
            CollocRatioWithPceRegressCollocRatioBP,
            CollocRatioWithPceRegressCollocRatioBPDN,
            CollocRatioWithPceRegressCollocRatioLars,
            CollocRatioWithPceRegressCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: CollocRatioWithPceRegressCollocRatioCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class CorePceExpansionOrderImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        CorePceExpansionOrderImportBuildPointsFileCustomAnnotated,
        CorePceExpansionOrderImportBuildPointsFileAnnotated,
        CorePceExpansionOrderImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=CorePceExpansionOrderImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "CorePceExpansionOrderImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    regression_method: (
        Union[
            CorePceExpansionOrderCollocPointsLeastSquares,
            CorePceExpansionOrderCollocPointsOMP,
            CorePceExpansionOrderCollocPointsBP,
            CorePceExpansionOrderCollocPointsBPDN,
            CorePceExpansionOrderCollocPointsLars,
            CorePceExpansionOrderCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: CorePceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class CorePceExpansionOrderCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    value: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    regression_method: (
        Union[
            CorePceExpansionOrderCollocRatioLeastSquares,
            CorePceExpansionOrderCollocRatioOMP,
            CorePceExpansionOrderCollocRatioBP,
            CorePceExpansionOrderCollocRatioBPDN,
            CorePceExpansionOrderCollocRatioLars,
            CorePceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: CorePceExpansionOrderCollocRatioCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class CorePceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        CorePceOrthogLeastInterpImportBuildPointsFileAnnotated,
        CorePceOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=CorePceOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "CorePceOrthogLeastInterpImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class MlpceAllocControlMixin(DakotaBaseModel):
    "Generated model for MlpceAllocControlMixin"

    allocation_control: (
        Union[
            MlpceAllocControlAllocationControlEstimatorVariance,
            MlpceAllocControlAllocationControlRipSampling,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Sample allocation approach for multilevel expansions",
        dakota={"union_pattern": 2},
    )


class PceBasisTypeMixin(DakotaBaseModel):
    "Generated model for PceBasisTypeMixin"

    basis_type: (
        Union[PceBasisTypeTensorProduct, PceBasisTypeTotalOrder, PceBasisTypeAdapted]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )


class PceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    regression_method: (
        Union[
            PceExpansionOrderCollocPointsLeastSquares,
            PceExpansionOrderCollocPointsOMP,
            PceExpansionOrderCollocPointsBP,
            PceExpansionOrderCollocPointsBPDN,
            PceExpansionOrderCollocPointsLars,
            PceExpansionOrderCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: PceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class PceExpansionOrderCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    value: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    regression_method: (
        Union[
            PceExpansionOrderCollocRatioLeastSquares,
            PceExpansionOrderCollocRatioOMP,
            PceExpansionOrderCollocPointsBP,
            PceExpansionOrderCollocPointsBPDN,
            PceExpansionOrderCollocPointsLars,
            PceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: PceExpansionOrderCollocPointsCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class PceExpansionOrderImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        PceExpansionOrderImportBuildPointsFileCustomAnnotated,
        PceExpansionOrderImportBuildPointsFileAnnotated,
        PceExpansionOrderImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=PceExpansionOrderImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "PceExpansionOrderImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class PceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceexpansionordersequencecollocratioconfig",
            list_field="collocation_points_sequence",
        ),
    ]

    sequence: DakotaFloat = DakotaField(
        gt=0,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    collocation_points_sequence: list[int] | None = DakotaField(
        default=None,
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "aliases": ["pilot_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ],
        },
    )
    regression_method: (
        Union[
            PceExpansionOrderSequenceCollocRatioLeastSquares,
            PceExpansionOrderSequenceCollocRatioOMP,
            PceExpansionOrderSequenceCollocRatioBP,
            PceExpansionOrderSequenceCollocRatioBPDN,
            PceExpansionOrderSequenceCollocRatioLars,
            PceExpansionOrderSequenceCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: PceExpansionOrderSequenceCollocRatioCV | None = DakotaField(
        default=None,
        description="Use cross validation to choose the 'best' polynomial order of a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.cross_validation",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    ratio_order: DakotaFloat = DakotaField(
        default=1.0,
        gt=0,
        description="Specify a non-linear the relationship between the expansion order of a polynomial chaos expansion and the number of samples that will be used to compute the PCE coefficients.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_ratio_terms_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    response_scaling: Literal[True] | None = DakotaField(
        default=None,
        description="Perform bounds-scaling on response values prior to surrogate emulation",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_scaling",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    use_derivatives: Literal[True] | None = DakotaField(
        default=None,
        description="Use derivative data to construct surrogate models",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.derivative_usage",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    tensor_grid: Literal[True] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    max_solver_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum iterations in determining polynomial coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_solver_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class PceExpansionOrderSequenceImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated,
        PceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        PceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=PceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "PceExpansionOrderSequenceImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated,
        PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFile(
    DakotaBaseModel
):
    "File containing points you wish to use to build a surrogate"

    filename: str = DakotaField(
        description="File containing points you wish to use to build a surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    format: Union[
        PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileAnnotated,
        PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileAnnotated",
        },
    )
    active_only: Literal[True] | None = DakotaField(
        default=None,
        description="Import only active variables from tabular data file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_active_only",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class PceRefinementMixin(DakotaBaseModel):
    "Generated model for PceRefinementMixin"

    p_refinement: (
        Union[PceRefinementPRefinementUniform, PceRefinementPRefinementDimAdaptive]
        | None
    ) = DakotaField(
        default=None,
        description="Automatic polynomial order refinement",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )
    max_refinement_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum number of expansion refinement iterations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_refinement_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class PceRegressionAlgSelectMixin(DakotaBaseModel):
    "Generated model for PceRegressionAlgSelectMixin"

    regression_method: (
        Union[
            PceRegressionAlgSelectLeastSquares,
            PceRegressionAlgSelectOMP,
            PceRegressionAlgSelectBP,
            PceRegressionAlgSelectBPDN,
            PceRegressionAlgSelectLars,
            PceRegressionAlgSelectLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )


class PceSGMixin(DakotaBaseModel):
    "Generated model for PceSGMixin"

    sparse_grid_level: PceSGSGLevel = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class PceSGSequenceMixin(DakotaBaseModel):
    "Generated model for PceSGSequenceMixin"

    sparse_grid_level_sequence: PceSGSequenceSGLevelSequence = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: BayesMfPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class BayesMfPceOrthogLeastInterpConfig(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfpceorthogleastinterpconfig",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="bayesmfpceorthogleastinterpconfig", list_field="tensor_grid"
        ),
    ]

    collocation_points_sequence: list[int] = DakotaField(
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: (
        BayesMfPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class BayesMlPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: BayesMlPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class BayesMlPceOrthogLeastInterpConfig(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmlpceorthogleastinterpconfig",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="bayesmlpceorthogleastinterpconfig", list_field="tensor_grid"
        ),
    ]

    collocation_points_sequence: list[int] = DakotaField(
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: (
        BayesMlPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class BayesPceExpansionOrderCollocPoints(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    collocation_points: BayesPceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class BayesPceExpansionOrderCollocPointsVariant13(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    collocation_points: BayesPceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class BayesPceExpansionOrderCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: BayesPceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class BayesPceExpansionOrderCollocRatioVariant5(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: BayesPceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class BayesPceOrthogLeastInterpConfig(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayespceorthogleastinterpconfig", list_field="tensor_grid"
        ),
    ]

    collocation_points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: BayesPceOrthogLeastInterpImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterpOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayespceorthogleastinterporthogleastinterp",
            list_field="tensor_grid",
        ),
    ]

    collocation_points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: (
        BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class CollocPtsWithPceRegressMixin(DakotaBaseModel):
    "Generated model for CollocPtsWithPceRegressMixin"

    collocation_points: CollocPtsWithPceRegressCollocPoints = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class CollocRatioPtsSequenceWithPceRegressMixin(DakotaBaseModel):
    "Generated model for CollocRatioPtsSequenceWithPceRegressMixin"

    collocation_ratio: CollocRatioPtsSequenceWithPceRegressCollocRatio = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class CollocRatioWithPceRegressMixin(DakotaBaseModel):
    "Generated model for CollocRatioWithPceRegressMixin"

    collocation_ratio: CollocRatioWithPceRegressCollocRatio = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class CorePceExpansionOrderCollocPoints(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    collocation_points: CorePceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class CorePceExpansionOrderCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: CorePceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class CorePceOrthogLeastInterpMixin(DakotaBaseModel):
    "Generated model for CorePceOrthogLeastInterpMixin"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="corepceorthogleastinterpmixin", list_field="tensor_grid"
        ),
    ]

    collocation_points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: CorePceOrthogLeastInterpImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )


class PceExpansionOrderCollocPoints(DakotaBaseModel):
    "Number of collocation points used to estimate expansion coefficients"

    collocation_points: PceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class PceExpansionOrderCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: PceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class PceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    "Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion."

    collocation_ratio: PceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class PceOrthogLeastInterpOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceorthogleastinterporthogleastinterp", list_field="tensor_grid"
        ),
    ]

    collocation_points: int = DakotaField(
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: (
        PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class PceOrthogLeastInterpSequenceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceorthogleastinterpsequenceorthogleastinterp",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="pceorthogleastinterpsequenceorthogleastinterp",
            list_field="tensor_grid",
        ),
    ]

    collocation_points_sequence: list[int] = DakotaField(
        description="Sequence of collocation point counts used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.collocation_points_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "SizetArray",
                }
            ]
        },
    )
    tensor_grid: list[int] | None = DakotaField(
        default=None,
        description="Use sub-sampled tensor-product quadrature points to build a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.tensor_grid_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    reuse_points: Literal[True] | None = DakotaField(
        default=None,
        description="This describes the behavior of reuse of points in constructing polynomial chaos expansion models.",
        dakota={
            "aliases": ["reuse_samples"],
            "materialization": [
                {
                    "ir_key": "method.nond.point_reuse",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "all",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: (
        PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class BayesMfPceExpansionOrderSequenceConfig(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfpceexpansionordersequenceconfig", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            BayesMfPceExpansionOrderSequenceBasisTypeTensorProduct,
            BayesMfPceExpansionOrderSequenceBasisTypeTotalOrder,
            BayesMfPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        BayesMfPceExpansionOrderSequenceCollocRatio,
        BayesMfPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        BayesMfPceExpansionOrderSequenceImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class BayesMfPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: BayesMfPceOrthogLeastInterpConfig = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMlPceExpansionOrderSequenceConfig(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmlpceexpansionordersequenceconfig", list_field="sequence"
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            BayesMlPceExpansionOrderSequenceBasisTypeTensorProduct,
            BayesMlPceExpansionOrderSequenceBasisTypeTotalOrder,
            BayesMlPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        BayesMlPceExpansionOrderSequenceCollocRatio,
        BayesMlPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        BayesMlPceExpansionOrderSequenceImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class BayesMlPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: BayesMlPceOrthogLeastInterpConfig = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceExpansionOrderConfig(DakotaBaseModel):
    "The (initial) order of a polynomial expansion"

    order: int = DakotaField(
        default=USHRT_MAX,
        description="The (initial) order of a polynomial expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            BayesPceExpansionOrderBasisTypeTensorProduct,
            BayesPceExpansionOrderBasisTypeTotalOrder,
            BayesPceExpansionOrderBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_selection: Union[
        BayesPceExpansionOrderCollocPoints,
        BayesPceExpansionOrderCollocRatio,
        BayesPceExpansionOrderExpansionSamples,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: BayesPceExpansionOrderImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceExpansionOrderExpansionOrder(DakotaBaseModel):
    "The (initial) order of a polynomial expansion"

    order: int = DakotaField(
        default=USHRT_MAX,
        description="The (initial) order of a polynomial expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            BayesPceExpansionOrderBasisTypeTensorProduct,
            BayesPceExpansionOrderBasisTypeTotalOrder,
            BayesPceExpansionOrderBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_selection: Union[
        BayesPceExpansionOrderCollocPoints,
        BayesPceExpansionOrderCollocRatio,
        BayesPceExpansionOrderExpansionSamples,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: BayesPceExpansionOrderImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )
    posterior_adaptive: Literal[True] | None = DakotaField(
        default=None,
        description="Adapt emulator model to increase accuracy in high posterior probability regions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.adaptive_posterior_refinement",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class BayesPceOrthogLeastInterp(DakotaBaseModel):
    "Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation."

    orthogonal_least_interpolation: BayesPceOrthogLeastInterpConfig = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesPceOrthogLeastInterpMixin(DakotaBaseModel):
    "Generated model for BayesPceOrthogLeastInterpMixin"

    orthogonal_least_interpolation: BayesPceOrthogLeastInterpOrthogLeastInterp = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class CorePceExpansionOrderMixin(DakotaBaseModel):
    "Generated model for CorePceExpansionOrderMixin"

    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            CorePceExpansionOrderBasisTypeTensorProduct,
            CorePceExpansionOrderBasisTypeTotalOrder,
            CorePceExpansionOrderBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    import_build_points_file: CorePceExpansionOrderImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )
    point_selection: Union[
        CorePceExpansionOrderCollocPoints,
        CorePceExpansionOrderCollocRatio,
        CorePceExpansionOrderExpansionSamples,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})


class PceExpansionOrderExpansionOrder(DakotaBaseModel):
    "The (initial) order of a polynomial expansion"

    order: int = DakotaField(
        default=USHRT_MAX,
        description="The (initial) order of a polynomial expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            PceExpansionOrderBasisTypeTensorProduct,
            PceExpansionOrderBasisTypeTotalOrder,
            PceExpansionOrderBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_selection: Union[
        PceExpansionOrderCollocPoints,
        PceExpansionOrderCollocRatio,
        PceExpansionOrderExpansionSamples,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: PceExpansionOrderImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )


class PceExpansionOrderSequenceExpansionOrderSequence(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceexpansionordersequenceexpansionordersequence",
            list_field="sequence",
        ),
    ]

    sequence: list[int] = DakotaField(
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_order_sequence",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
    dimension_preference: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="A set of weights specifying the realtive importance of each uncertain variable (dimension)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.dimension_preference",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    basis_type: (
        Union[
            PceExpansionOrderSequenceBasisTypeTensorProduct,
            PceExpansionOrderSequenceBasisTypeTotalOrder,
            PceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        PceExpansionOrderSequenceCollocRatio,
        PceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: PceExpansionOrderSequenceImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )


class PceOrthogLeastInterpMixin(DakotaBaseModel):
    "Generated model for PceOrthogLeastInterpMixin"

    orthogonal_least_interpolation: PceOrthogLeastInterpOrthogLeastInterp = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class PceOrthogLeastInterpSequenceMixin(DakotaBaseModel):
    "Generated model for PceOrthogLeastInterpSequenceMixin"

    orthogonal_least_interpolation: PceOrthogLeastInterpSequenceOrthogLeastInterp = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class BayesMfPceExpansionOrderSequence(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    expansion_order_sequence: BayesMfPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMlPceExpansionOrderSequence(DakotaBaseModel):
    "Sequence of expansion orders used in a multi-stage expansion"

    expansion_order_sequence: BayesMlPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesPceExpansionOrder(DakotaBaseModel):
    "The (initial) order of a polynomial expansion"

    expansion_order: BayesPceExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class BayesPceExpansionOrderMixin(DakotaBaseModel):
    "Generated model for BayesPceExpansionOrderMixin"

    expansion_order: BayesPceExpansionOrderExpansionOrder = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class PceExpansionOrderMixin(DakotaBaseModel):
    "Generated model for PceExpansionOrderMixin"

    expansion_order: PceExpansionOrderExpansionOrder = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class PceExpansionOrderSequenceMixin(DakotaBaseModel):
    "Generated model for PceExpansionOrderSequenceMixin"

    expansion_order_sequence: PceExpansionOrderSequenceExpansionOrderSequence = (
        DakotaField(
            default=...,
            description="Sequence of expansion orders used in a multi-stage expansion",
            dakota={"argument": "sequence"},
        )
    )


class BayesMfPceMfPce(DakotaBaseModel):
    "Multifidelity Polynomial Chaos Expansion as an emulator model."

    p_refinement: (
        Union[MfPcePRefinementUniform, BayesMfPcePRefinementDimAdaptive] | None
    ) = DakotaField(
        default=None,
        description="Automatic polynomial order refinement",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )
    max_refinement_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum number of expansion refinement iterations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_refinement_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    allocation_control: Union[BayesMfPceAllocationControlGreedy] | None = DakotaField(
        default=None,
        description="Sample allocation approach for multifidelity expansions",
        dakota={"union_pattern": 2},
    )
    discrepancy_emulation: (
        Union[MfPceDiscrepancyEmulationDistinct, MfPceDiscrepancyEmulationRecursive]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    coefficient_approach: Union[
        BayesMfPceQuadratureOrderSequence,
        BayesMfPceSGLevelSequence,
        BayesMfPceExpansionOrderSequence,
        BayesMfPceOrthogLeastInterp,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[MfPceAskey, MfPceWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )
    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_type: Union[MfPceDiagCov, MfPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMlPceMlPce(DakotaBaseModel):
    "Multilevel Polynomial Chaos Expansion as an emulator model."

    allocation_control: (
        Union[
            BayesMlPceAllocationControlEstimatorVariance,
            BayesMlPceAllocationControlRipSampling,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Sample allocation approach for multilevel expansions",
        dakota={"union_pattern": 2},
    )
    discrepancy_emulation: (
        Union[
            BayesMlPceDiscrepancyEmulationDistinct,
            BayesMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    coefficient_approach: Union[
        BayesMlPceExpansionOrderSequence, BayesMlPceOrthogLeastInterp
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[MlPceAskey, MlPceWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )
    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_type: Union[MlPceDiagCov, MlPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesPcePce(DakotaBaseModel):
    "Polynomial Chaos Expansion surrogate model"

    p_refinement: (
        Union[BayesPcePRefinementUniform, BayesPcePRefinementDimAdaptive] | None
    ) = DakotaField(
        default=None,
        description="Automatic polynomial order refinement",
        dakota={
            "union_pattern": 2,
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )
    max_refinement_iterations: int = DakotaField(
        default=SZ_MAX,
        ge=0,
        description="Maximum number of expansion refinement iterations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.max_refinement_iterations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )
    coefficient_approach: Union[
        BayesPceQuadratureOrder,
        BayesPceSGLevel,
        BayesPceCubatureIntegrand,
        BayesPceExpansionOrder,
        BayesPceOrthogLeastInterp,
        BayesPceImportExpansionFile,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[BayesPceAskey, BayesPceWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
    )
    normalized: Literal[True] | None = DakotaField(
        default=None,
        description="The normalized specification requests output of PCE coefficients that correspond to normalized orthogonal basis polynomials",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.normalized",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    export_expansion_file: str | None = DakotaField(
        default=None,
        description="Export the coefficients and multi-index of a Polynomial Chaos Expansion (PCE) to a file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.export_expansion_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_type: Union[BayesPceDiagCov, BayesPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfPceMixin(DakotaBaseModel):
    "Generated model for BayesMfPceMixin"

    mf_pce: BayesMfPceMfPce = DakotaField(
        description="Multifidelity Polynomial Chaos Expansion as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MF_PCE_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMlPceMixin(DakotaBaseModel):
    "Generated model for BayesMlPceMixin"

    ml_pce: BayesMlPceMlPce = DakotaField(
        description="Multilevel Polynomial Chaos Expansion as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ML_PCE_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesPceMixin(DakotaBaseModel):
    "Generated model for BayesPceMixin"

    pce: BayesPcePce = DakotaField(
        description="Polynomial Chaos Expansion surrogate model",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PCE_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )
