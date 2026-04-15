"""Generated Pydantic models for shared.expansion/pce"""

from __future__ import annotations

from ....base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ....base import SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ....validation import ValidationRule
from ....validation.rules import CheckNonnegativeList, CompareLength, CompareLengthOne


class PceCubatureMixin(DakotaBaseModel):
    """Generated model for PceCubatureMixin"""

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
    """Generated model for PceExpansionImportMixin"""

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
    """Generated model for MfPcePRefinementUniform"""

    uniform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPcePRefinementDimAdaptiveSobol"""

    sobol: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPcePRefinementDimAdaptiveDecay"""

    decay: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPcePRefinementDimAdaptiveGeneralized"""

    generalized: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceAllocationControlGreedy"""

    greedy: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPceDiscrepancyEmulationDistinct"""

    distinct: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPceDiscrepancyEmulationRecursive"""

    recursive: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceQuadratureOrderSequenceNested"""

    nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceQuadratureOrderSequenceNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceSGLevelSequenceRestricted"""

    restricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceSGLevelSequenceUnrestricted"""

    unrestricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceBasisTypeAdaptedConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioOMPConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioLassoConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioCV"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceExpansionSamplesSequenceConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPceExpansionOrderSequenceImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPceAskey"""

    askey: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPceWiener"""

    wiener: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPceDiagCov"""

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MfPceFullCov"""

    full_covariance: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceAllocationControlEstimatorVarianceConfig"""

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
    """Generated model for BayesMlPceAllocationControlRipSampling"""

    rip_sampling: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceDiscrepancyEmulationDistinct"""

    distinct: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceDiscrepancyEmulationRecursive"""

    recursive: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceBasisTypeAdaptedConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioOMPConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioLassoConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioCV"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMlPceExpansionOrderSequenceImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MlPceAskey"""

    askey: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MlPceWiener"""

    wiener: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MlPceDiagCov"""

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MlPceFullCov"""

    full_covariance: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPcePRefinementUniform"""

    uniform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPcePRefinementDimAdaptiveSobol"""

    sobol: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPcePRefinementDimAdaptiveDecay"""

    decay: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPcePRefinementDimAdaptiveGeneralized"""

    generalized: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceQuadratureOrderNested"""

    nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceQuadratureOrderNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceSGLevelRestricted"""

    restricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceSGLevelUnrestricted"""

    unrestricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceCubatureIntegrand"""

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
    """Generated model for BayesPceExpansionOrderBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderBasisTypeAdaptedConfig"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderCollocPointsLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderCollocPointsOMPConfig"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderCollocPointsLassoConfig"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsCV"""

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
    """Generated model for BayesPceExpansionOrderExpansionSamplesConfig"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceImportExpansionFile"""

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
    """Generated model for BayesPceAskey"""

    askey: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceWiener"""

    wiener: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceDiagCov"""

    diagonal_covariance: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceFullCov"""

    full_covariance: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderBasisTypeTensorProductVariant1"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderBasisTypeTotalOrderVariant1"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderBasisTypeAdaptedConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLeastSquaresSvdVariant1"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderCollocPointsLeastSquaresEqConVariant1"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderCollocPointsOMPConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsBPVariant2"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderCollocPointsLassoConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsCVVariant1"""

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
    """Generated model for BayesPceExpansionOrderExpansionSamplesConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileAnnotatedVariant1"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileFreeformVariant1"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocPtsWithPceRegressCollocPointsLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocPtsWithPceRegressCollocPointsLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocPtsWithPceRegressCollocPointsOMPConfig"""

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
    """Generated model for CollocPtsWithPceRegressCollocPointsBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocPtsWithPceRegressCollocPointsLassoConfig"""

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
    """Generated model for CollocPtsWithPceRegressCollocPointsCV"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioOMPConfig"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioLassoConfig"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioCV"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocRatioWithPceRegressCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocRatioWithPceRegressCollocRatioOMPConfig"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CollocRatioWithPceRegressCollocRatioLassoConfig"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioCV"""

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
    """Generated model for CorePceExpansionOrderBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderBasisTypeAdaptedConfig"""

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
    """Generated model for CorePceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for CorePceExpansionOrderImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocPointsLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocPointsLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocPointsOMPConfig"""

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
    """Generated model for CorePceExpansionOrderCollocPointsBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocPointsBPDNConfig"""

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
    """Generated model for CorePceExpansionOrderCollocPointsLarsConfig"""

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
    """Generated model for CorePceExpansionOrderCollocPointsLassoConfig"""

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
    """Generated model for CorePceExpansionOrderCollocPointsCV"""

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
    """Generated model for CorePceExpansionOrderCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocRatioOMPConfig"""

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
    """Generated model for CorePceExpansionOrderCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceExpansionOrderCollocRatioBPDNConfig"""

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
    """Generated model for CorePceExpansionOrderCollocRatioLarsConfig"""

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
    """Generated model for CorePceExpansionOrderCollocRatioLassoConfig"""

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
    """Generated model for CorePceExpansionOrderCollocRatioCV"""

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
    """Generated model for CorePceExpansionOrderExpansionSamplesConfig"""

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
    """Generated model for CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for CorePceOrthogLeastInterpImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for CorePceOrthogLeastInterpImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MlpceAllocControlAllocationControlEstimatorVarianceConfig"""

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
    """Generated model for MlpceAllocControlAllocationControlRipSampling"""

    rip_sampling: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceBasisTypeAdaptedConfig"""

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
    """Generated model for PceExpansionOrderBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderBasisTypeAdaptedConfig"""

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
    """Generated model for PceExpansionOrderCollocPointsLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderCollocPointsLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderCollocPointsOMPConfig"""

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
    """Generated model for PceExpansionOrderCollocPointsBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderCollocPointsLassoConfig"""

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
    """Generated model for PceExpansionOrderCollocPointsCV"""

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
    """Generated model for PceExpansionOrderExpansionSamplesConfig"""

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
    """Generated model for PceExpansionOrderImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for PceExpansionOrderImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceBasisTypeAdaptedConfig"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceCollocRatioOMPConfig"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceCollocRatioLassoConfig"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioCV"""

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
    """Generated model for PceExpansionOrderSequenceExpansionSamplesSequenceConfig"""

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
    """Generated model for PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for PceExpansionOrderSequenceImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionOrderSequenceImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceExpansionSamplesExpansionSamples"""

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
    """Generated model for PceExpansionSamplesSequenceExpansionSamplesSequence"""

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
    """Generated model for PceOptionsAskey"""

    askey: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceOptionsWiener"""

    wiener: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig"""

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
    """Generated model for PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRefinementPRefinementUniform"""

    uniform: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRefinementPRefinementDimAdaptiveSobol"""

    sobol: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRefinementPRefinementDimAdaptiveDecay"""

    decay: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRefinementPRefinementDimAdaptiveGeneralized"""

    generalized: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRegressionAlgSelectLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRegressionAlgSelectLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRegressionAlgSelectOMPConfig"""

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
    """Generated model for PceRegressionAlgSelectBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceRegressionAlgSelectBPDNConfig"""

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
    """Generated model for PceRegressionAlgSelectLarsConfig"""

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
    """Generated model for PceRegressionAlgSelectLassoConfig"""

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
    """Generated model for PceRegressionOptsCV"""

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
    """Generated model for PceSGLevelRestricted"""

    restricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGLevelUnrestricted"""

    unrestricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGLevelNested"""

    nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGLevelNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGSequenceLevelSequenceRestricted"""

    restricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGSequenceLevelSequenceUnrestricted"""

    unrestricted: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGSequenceLevelSequenceNested"""

    nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for PceSGSequenceLevelSequenceNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for BayesMfPcePRefinementDimAdaptive"""

    dimension_adaptive: Union[
        MfPcePRefinementDimAdaptiveSobol,
        MfPcePRefinementDimAdaptiveDecay,
        MfPcePRefinementDimAdaptiveGeneralized,
    ]


class BayesMfPceQuadratureOrderSequenceConfig(DakotaBaseModel):
    """Generated model for BayesMfPceQuadratureOrderSequenceConfig"""

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
    """Generated model for BayesMfPceSGLevelSequenceConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceBasisTypeAdapted"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioLeastSquares"""

    least_squares: Union[
        BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        BayesMfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class BayesMfPceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioOMP"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioBPDN"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioLars"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioLasso"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceExpansionSamplesSequence"""

    expansion_samples_sequence: BayesMfPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    """Generated model for BayesMfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for BayesMfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for BayesMlPceAllocationControlEstimatorVariance"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceBasisTypeAdapted"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioLeastSquares"""

    least_squares: Union[
        BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        BayesMlPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class BayesMlPceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioOMP"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioBPDN"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioLars"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioLasso"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceExpansionSamplesSequence"""

    expansion_samples_sequence: BayesMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(
    DakotaBaseModel
):
    """Generated model for BayesMlPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for BayesMlPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for BayesPcePRefinementDimAdaptive"""

    dimension_adaptive: Union[
        BayesPcePRefinementDimAdaptiveSobol,
        BayesPcePRefinementDimAdaptiveDecay,
        BayesPcePRefinementDimAdaptiveGeneralized,
    ]


class BayesPceQuadratureOrderConfig(DakotaBaseModel):
    """Generated model for BayesPceQuadratureOrderConfig"""

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
    """Generated model for BayesPceSGLevelConfig"""

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
    """Generated model for BayesPceExpansionOrderBasisTypeAdapted"""

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
    """Generated model for BayesPceExpansionOrderBasisTypeAdaptedVariant2"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLeastSquares"""

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class BayesPceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocRatioLeastSquares"""

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class BayesPceExpansionOrderCollocPointsLeastSquaresVariant3(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocPointsLeastSquaresVariant3"""

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class BayesPceExpansionOrderCollocRatioLeastSquaresVariant1(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocRatioLeastSquaresVariant1"""

    least_squares: Union[
        BayesPceExpansionOrderCollocPointsLeastSquaresSvd,
        BayesPceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class BayesPceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocPointsOMP"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsBPDN"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLars"""

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
    """Generated model for BayesPceExpansionOrderCollocRatioOMP"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsOMPVariant2"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsBPDNVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLarsVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocRatioOMPVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLasso"""

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
    """Generated model for BayesPceExpansionOrderCollocRatioLasso"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsLassoVariant2"""

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
    """Generated model for BayesPceExpansionOrderCollocRatioLassoVariant1"""

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
    """Generated model for BayesPceExpansionOrderExpansionSamples"""

    expansion_samples: BayesPceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class BayesPceExpansionOrderExpansionSamplesVariant2(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderExpansionSamplesVariant2"""

    expansion_samples: BayesPceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class BayesPceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for BayesPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileCustomAnnotatedVariant2"""

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
    """Generated model for BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for CollocPtsWithPceRegressCollocPointsLeastSquares"""

    least_squares: Union[
        CollocPtsWithPceRegressCollocPointsLeastSquaresSvd,
        CollocPtsWithPceRegressCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class CollocPtsWithPceRegressCollocPointsOMP(DakotaBaseModel):
    """Generated model for CollocPtsWithPceRegressCollocPointsOMP"""

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
    """Generated model for CollocPtsWithPceRegressCollocPointsBPDN"""

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
    """Generated model for CollocPtsWithPceRegressCollocPointsLars"""

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
    """Generated model for CollocPtsWithPceRegressCollocPointsLasso"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquares"""

    least_squares: Union[
        CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresSvd,
        CollocRatioPtsSequenceWithPceRegressCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class CollocRatioPtsSequenceWithPceRegressCollocRatioOMP(DakotaBaseModel):
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioOMP"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioBPDN"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioLars"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatioLasso"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioLeastSquares"""

    least_squares: Union[
        CollocRatioWithPceRegressCollocRatioLeastSquaresSvd,
        CollocRatioWithPceRegressCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class CollocRatioWithPceRegressCollocRatioOMP(DakotaBaseModel):
    """Generated model for CollocRatioWithPceRegressCollocRatioOMP"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioBPDN"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioLars"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatioLasso"""

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
    """Generated model for CorePceExpansionOrderBasisTypeAdapted"""

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
    """Generated model for CorePceExpansionOrderImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for CorePceExpansionOrderCollocPointsLeastSquares"""

    least_squares: Union[
        CorePceExpansionOrderCollocPointsLeastSquaresSvd,
        CorePceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class CorePceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    """Generated model for CorePceExpansionOrderCollocPointsOMP"""

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
    """Generated model for CorePceExpansionOrderCollocPointsBPDN"""

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
    """Generated model for CorePceExpansionOrderCollocPointsLars"""

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
    """Generated model for CorePceExpansionOrderCollocPointsLasso"""

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
    """Generated model for CorePceExpansionOrderCollocRatioLeastSquares"""

    least_squares: Union[
        CorePceExpansionOrderCollocRatioLeastSquaresSvd,
        CorePceExpansionOrderCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class CorePceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    """Generated model for CorePceExpansionOrderCollocRatioOMP"""

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
    """Generated model for CorePceExpansionOrderCollocRatioBPDN"""

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
    """Generated model for CorePceExpansionOrderCollocRatioLars"""

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
    """Generated model for CorePceExpansionOrderCollocRatioLasso"""

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
    """Generated model for CorePceExpansionOrderExpansionSamples"""

    expansion_samples: CorePceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for CorePceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for MlpceAllocControlAllocationControlEstimatorVariance"""

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
    """Generated model for PceBasisTypeAdapted"""

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
    """Generated model for PceExpansionOrderBasisTypeAdapted"""

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
    """Generated model for PceExpansionOrderCollocPointsLeastSquares"""

    least_squares: Union[
        PceExpansionOrderCollocPointsLeastSquaresSvd,
        PceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class PceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocRatioLeastSquares"""

    least_squares: Union[
        PceExpansionOrderCollocPointsLeastSquaresSvd,
        PceExpansionOrderCollocPointsLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class PceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocPointsOMP"""

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
    """Generated model for PceExpansionOrderCollocPointsBPDN"""

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
    """Generated model for PceExpansionOrderCollocPointsLars"""

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
    """Generated model for PceExpansionOrderCollocRatioOMP"""

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
    """Generated model for PceExpansionOrderCollocPointsLasso"""

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
    """Generated model for PceExpansionOrderCollocRatioLasso"""

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
    """Generated model for PceExpansionOrderExpansionSamples"""

    expansion_samples: PceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class PceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for PceExpansionOrderImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for PceExpansionOrderSequenceBasisTypeAdapted"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioLeastSquares"""

    least_squares: Union[
        PceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        PceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class PceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    """Generated model for PceExpansionOrderSequenceCollocRatioOMP"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioBPDN"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioLars"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioLasso"""

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
    """Generated model for PceExpansionOrderSequenceExpansionSamplesSequence"""

    expansion_samples_sequence: PceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for PceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for PceExpansionSamplesMixin"""

    expansion_samples: PceExpansionSamplesExpansionSamples = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class PceExpansionSamplesSequenceMixin(DakotaBaseModel):
    """Generated model for PceExpansionSamplesSequenceMixin"""

    expansion_samples_sequence: PceExpansionSamplesSequenceExpansionSamplesSequence = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class PceOptionsMixin(DakotaBaseModel):
    """Generated model for PceOptionsMixin"""

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
    """Generated model for PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

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
    """Generated model for PceRefinementPRefinementDimAdaptive"""

    dimension_adaptive: Union[
        PceRefinementPRefinementDimAdaptiveSobol,
        PceRefinementPRefinementDimAdaptiveDecay,
        PceRefinementPRefinementDimAdaptiveGeneralized,
    ]


class PceRegressionAlgSelectLeastSquares(DakotaBaseModel):
    """Generated model for PceRegressionAlgSelectLeastSquares"""

    least_squares: Union[
        PceRegressionAlgSelectLeastSquaresSvd,
        PceRegressionAlgSelectLeastSquaresEqCon,
        dict,
    ] = DakotaField(
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
        }
    )


class PceRegressionAlgSelectOMP(DakotaBaseModel):
    """Generated model for PceRegressionAlgSelectOMP"""

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
    """Generated model for PceRegressionAlgSelectBPDN"""

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
    """Generated model for PceRegressionAlgSelectLars"""

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
    """Generated model for PceRegressionAlgSelectLasso"""

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
    """Generated model for PceRegressionOptsMixin"""

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
    """Generated model for PceSGSGLevel"""

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
    """Generated model for PceSGSequenceSGLevelSequence"""

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
    """Generated model for BayesMfPceQuadratureOrderSequence"""

    quadrature_order_sequence: BayesMfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfPceSGLevelSequence(DakotaBaseModel):
    """Generated model for BayesMfPceSGLevelSequence"""

    sparse_grid_level_sequence: BayesMfPceSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatioConfig"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceImportBuildPointsFile"""

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
    """Generated model for BayesMfPceOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatioConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceImportBuildPointsFile"""

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
    """Generated model for BayesMlPceOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for BayesPceQuadratureOrder"""

    quadrature_order: BayesPceQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class BayesPceSGLevel(DakotaBaseModel):
    """Generated model for BayesPceSGLevel"""

    sparse_grid_level: BayesPceSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class BayesPceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocPointsConfig"""

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
    """Generated model for BayesPceExpansionOrderCollocPointsConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderCollocRatioConfig"""

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
    """Generated model for BayesPceExpansionOrderCollocRatioConfigVariant1"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFile"""

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
    """Generated model for BayesPceExpansionOrderImportBuildPointsFileVariant5"""

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
    """Generated model for BayesPceOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for BayesPceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for CollocPtsWithPceRegressCollocPoints"""

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
    """Generated model for CollocRatioPtsSequenceWithPceRegressCollocRatio"""

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
    """Generated model for CollocRatioWithPceRegressCollocRatio"""

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
    """Generated model for CorePceExpansionOrderImportBuildPointsFile"""

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
    """Generated model for CorePceExpansionOrderCollocPointsConfig"""

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
    """Generated model for CorePceExpansionOrderCollocRatioConfig"""

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
    """Generated model for CorePceOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for MlpceAllocControlMixin"""

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
    """Generated model for PceBasisTypeMixin"""

    basis_type: (
        Union[PceBasisTypeTensorProduct, PceBasisTypeTotalOrder, PceBasisTypeAdapted]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )


class PceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocPointsConfig"""

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
    """Generated model for PceExpansionOrderCollocRatioConfig"""

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
    """Generated model for PceExpansionOrderImportBuildPointsFile"""

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
    """Generated model for PceExpansionOrderSequenceCollocRatioConfig"""

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
    """Generated model for PceExpansionOrderSequenceImportBuildPointsFile"""

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
    """Generated model for PceOrthogLeastInterpOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for PceOrthogLeastInterpSequenceOrthogLeastInterpImportBuildPointsFile"""

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
    """Generated model for PceRefinementMixin"""

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
    """Generated model for PceRegressionAlgSelectMixin"""

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
    """Generated model for PceSGMixin"""

    sparse_grid_level: PceSGSGLevel = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class PceSGSequenceMixin(DakotaBaseModel):
    """Generated model for PceSGSequenceMixin"""

    sparse_grid_level_sequence: PceSGSequenceSGLevelSequence = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    """Generated model for BayesMfPceExpansionOrderSequenceCollocRatio"""

    collocation_ratio: BayesMfPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class BayesMfPceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for BayesMfPceOrthogLeastInterpConfig"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceCollocRatio"""

    collocation_ratio: BayesMlPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class BayesMlPceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for BayesMlPceOrthogLeastInterpConfig"""

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
    """Generated model for BayesPceExpansionOrderCollocPoints"""

    collocation_points: BayesPceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class BayesPceExpansionOrderCollocPointsVariant13(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocPointsVariant13"""

    collocation_points: BayesPceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class BayesPceExpansionOrderCollocRatio(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocRatio"""

    collocation_ratio: BayesPceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class BayesPceExpansionOrderCollocRatioVariant5(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderCollocRatioVariant5"""

    collocation_ratio: BayesPceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class BayesPceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for BayesPceOrthogLeastInterpConfig"""

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
    """Generated model for BayesPceOrthogLeastInterpOrthogLeastInterp"""

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
    """Generated model for CollocPtsWithPceRegressMixin"""

    collocation_points: CollocPtsWithPceRegressCollocPoints = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class CollocRatioPtsSequenceWithPceRegressMixin(DakotaBaseModel):
    """Generated model for CollocRatioPtsSequenceWithPceRegressMixin"""

    collocation_ratio: CollocRatioPtsSequenceWithPceRegressCollocRatio = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class CollocRatioWithPceRegressMixin(DakotaBaseModel):
    """Generated model for CollocRatioWithPceRegressMixin"""

    collocation_ratio: CollocRatioWithPceRegressCollocRatio = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class CorePceExpansionOrderCollocPoints(DakotaBaseModel):
    """Generated model for CorePceExpansionOrderCollocPoints"""

    collocation_points: CorePceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class CorePceExpansionOrderCollocRatio(DakotaBaseModel):
    """Generated model for CorePceExpansionOrderCollocRatio"""

    collocation_ratio: CorePceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class CorePceOrthogLeastInterpMixin(DakotaBaseModel):
    """Generated model for CorePceOrthogLeastInterpMixin"""

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
    """Generated model for PceExpansionOrderCollocPoints"""

    collocation_points: PceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class PceExpansionOrderCollocRatio(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocRatio"""

    collocation_ratio: PceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class PceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    """Generated model for PceExpansionOrderSequenceCollocRatio"""

    collocation_ratio: PceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class PceOrthogLeastInterpOrthogLeastInterp(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpOrthogLeastInterp"""

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
    """Generated model for PceOrthogLeastInterpSequenceOrthogLeastInterp"""

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
    """Generated model for BayesMfPceExpansionOrderSequenceConfig"""

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
    """Generated model for BayesMfPceOrthogLeastInterp"""

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
    """Generated model for BayesMlPceExpansionOrderSequenceConfig"""

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
    """Generated model for BayesMlPceOrthogLeastInterp"""

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
    """Generated model for BayesPceExpansionOrderConfig"""

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
    """Generated model for BayesPceExpansionOrderExpansionOrder"""

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
    """Generated model for BayesPceOrthogLeastInterp"""

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
    """Generated model for BayesPceOrthogLeastInterpMixin"""

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
    """Generated model for CorePceExpansionOrderMixin"""

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
    """Generated model for PceExpansionOrderExpansionOrder"""

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
    """Generated model for PceExpansionOrderSequenceExpansionOrderSequence"""

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
    """Generated model for PceOrthogLeastInterpMixin"""

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
    """Generated model for PceOrthogLeastInterpSequenceMixin"""

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
    """Generated model for BayesMfPceExpansionOrderSequence"""

    expansion_order_sequence: BayesMfPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMlPceExpansionOrderSequence(DakotaBaseModel):
    """Generated model for BayesMlPceExpansionOrderSequence"""

    expansion_order_sequence: BayesMlPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesPceExpansionOrder(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrder"""

    expansion_order: BayesPceExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class BayesPceExpansionOrderMixin(DakotaBaseModel):
    """Generated model for BayesPceExpansionOrderMixin"""

    expansion_order: BayesPceExpansionOrderExpansionOrder = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class PceExpansionOrderMixin(DakotaBaseModel):
    """Generated model for PceExpansionOrderMixin"""

    expansion_order: PceExpansionOrderExpansionOrder = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class PceExpansionOrderSequenceMixin(DakotaBaseModel):
    """Generated model for PceExpansionOrderSequenceMixin"""

    expansion_order_sequence: PceExpansionOrderSequenceExpansionOrderSequence = (
        DakotaField(
            default=...,
            description="Sequence of expansion orders used in a multi-stage expansion",
            dakota={"argument": "sequence"},
        )
    )


class BayesMfPceMfPce(DakotaBaseModel):
    """Generated model for BayesMfPceMfPce"""

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
    """Generated model for BayesMlPceMlPce"""

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
    """Generated model for BayesPcePce"""

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
    """Generated model for BayesMfPceMixin"""

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
    """Generated model for BayesMlPceMixin"""

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
    """Generated model for BayesPceMixin"""

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
