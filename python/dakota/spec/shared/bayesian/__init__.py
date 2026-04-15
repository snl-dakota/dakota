"""Generated Pydantic models for shared.bayesian"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField, DakotaFloat
from typing import Literal, Union
from ...base import INT_MAX, SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ...validation import ValidationRule
from ...validation.rules import CheckNonnegativeList, CompareLength, CompareLengthOne


class EmulatorGPSurfpack(DakotaBaseModel):
    """Generated model for EmulatorGPSurfpack"""

    surfpack: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "KRIGING_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorGPDakota(DakotaBaseModel):
    """Generated model for EmulatorGPDakota"""

    dakota: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GP_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class GPImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for GPImportBuildPointsFileCustomAnnotatedConfig"""

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


class GPImportBuildPointsFileAnnotated(DakotaBaseModel):
    """Generated model for GPImportBuildPointsFileAnnotated"""

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


class GPImportBuildPointsFileFreeform(DakotaBaseModel):
    """Generated model for GPImportBuildPointsFileFreeform"""

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


class EmulatorPcePRefinementUniform(DakotaBaseModel):
    """Generated model for EmulatorPcePRefinementUniform"""

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


class EmulatorPcePRefinementDimAdaptiveSobol(DakotaBaseModel):
    """Generated model for EmulatorPcePRefinementDimAdaptiveSobol"""

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


class EmulatorPcePRefinementDimAdaptiveDecay(DakotaBaseModel):
    """Generated model for EmulatorPcePRefinementDimAdaptiveDecay"""

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


class EmulatorPcePRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    """Generated model for EmulatorPcePRefinementDimAdaptiveGeneralized"""

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


class EmulatorPceQuadratureOrderNested(DakotaBaseModel):
    """Generated model for EmulatorPceQuadratureOrderNested"""

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


class EmulatorPceQuadratureOrderNonNested(DakotaBaseModel):
    """Generated model for EmulatorPceQuadratureOrderNonNested"""

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


class EmulatorPceSGLevelRestricted(DakotaBaseModel):
    """Generated model for EmulatorPceSGLevelRestricted"""

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


class EmulatorPceSGLevelUnrestricted(DakotaBaseModel):
    """Generated model for EmulatorPceSGLevelUnrestricted"""

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


class EmulatorPceCubatureIntegrand(DakotaBaseModel):
    """Generated model for EmulatorPceCubatureIntegrand"""

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


class EmulatorPceExpansionOrderBasisTypeTensorProduct(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderBasisTypeTensorProduct"""

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


class EmulatorPceExpansionOrderBasisTypeTotalOrder(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderBasisTypeTotalOrder"""

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


class EmulatorPceExpansionOrderBasisTypeAdaptedConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderBasisTypeAdaptedConfig"""

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


class EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd"""

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


class EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon"""

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


class EmulatorPceExpansionOrderCollocPointsOMPConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsOMPConfig"""

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


class EmulatorPceExpansionOrderCollocPointsBP(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsBP"""

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


class EmulatorPceExpansionOrderCollocPointsLassoConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsLassoConfig"""

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


class EmulatorPceExpansionOrderCollocPointsCV(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsCV"""

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


class EmulatorPceExpansionOrderExpansionSamplesConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderExpansionSamplesConfig"""

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


class EmulatorPceImportExpansionFile(DakotaBaseModel):
    """Generated model for EmulatorPceImportExpansionFile"""

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


class EmulatorPceAskey(DakotaBaseModel):
    """Generated model for EmulatorPceAskey"""

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


class EmulatorPceWiener(DakotaBaseModel):
    """Generated model for EmulatorPceWiener"""

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


class EmulatorPceDiagCov(DakotaBaseModel):
    """Generated model for EmulatorPceDiagCov"""

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


class EmulatorPceFullCov(DakotaBaseModel):
    """Generated model for EmulatorPceFullCov"""

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


class EmulatorMlPceAllocationControlEstimatorVarianceConfig(DakotaBaseModel):
    """Generated model for EmulatorMlPceAllocationControlEstimatorVarianceConfig"""

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


class EmulatorMlPceAllocationControlRipSampling(DakotaBaseModel):
    """Generated model for EmulatorMlPceAllocationControlRipSampling"""

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


class EmulatorMlPceDiscrepancyEmulationDistinct(DakotaBaseModel):
    """Generated model for EmulatorMlPceDiscrepancyEmulationDistinct"""

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


class EmulatorMlPceDiscrepancyEmulationRecursive(DakotaBaseModel):
    """Generated model for EmulatorMlPceDiscrepancyEmulationRecursive"""

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


class EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig(
    DakotaBaseModel
):
    """Generated model for EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceexpansionordersequenceexpansionsamplessequenceconfig",
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


class EmulatorMfPceAllocationControlGreedy(DakotaBaseModel):
    """Generated model for EmulatorMfPceAllocationControlGreedy"""

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


class EmulatorScHRefinementLocalAdaptive(DakotaBaseModel):
    """Generated model for EmulatorScHRefinementLocalAdaptive"""

    local_adaptive: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_ADAPTIVE_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorScSGLevelNodal(DakotaBaseModel):
    """Generated model for EmulatorScSGLevelNodal"""

    nodal: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NODAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorScSGLevelHierarchical(DakotaBaseModel):
    """Generated model for EmulatorScSGLevelHierarchical"""

    hierarchical: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorScPiecewise(DakotaBaseModel):
    """Generated model for EmulatorScPiecewise"""

    piecewise: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "METHOD_PIECEWISE",
                    "stored_value": "STD_UNIFORM_U",
                    "secondary_ir_key": "method.nond.piecewise_basis",
                    "secondary_ir_value_type": "bool",
                    "secondary_literal_value": True,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfScPRefinementUniform(DakotaBaseModel):
    """Generated model for MfScPRefinementUniform"""

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


class MfScPRefinementDimAdaptiveSobol(DakotaBaseModel):
    """Generated model for MfScPRefinementDimAdaptiveSobol"""

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


class MfScPRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    """Generated model for MfScPRefinementDimAdaptiveGeneralized"""

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


class MfScHRefinementLocalAdaptive(DakotaBaseModel):
    """Generated model for MfScHRefinementLocalAdaptive"""

    local_adaptive: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_ADAPTIVE_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfScAllocationControlGreedy(DakotaBaseModel):
    """Generated model for MfScAllocationControlGreedy"""

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


class MfScDiscrepancyEmulationDistinct(DakotaBaseModel):
    """Generated model for MfScDiscrepancyEmulationDistinct"""

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


class MfScDiscrepancyEmulationRecursive(DakotaBaseModel):
    """Generated model for MfScDiscrepancyEmulationRecursive"""

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


class MfScQuadratureOrderSequenceNested(DakotaBaseModel):
    """Generated model for MfScQuadratureOrderSequenceNested"""

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


class MfScQuadratureOrderSequenceNonNested(DakotaBaseModel):
    """Generated model for MfScQuadratureOrderSequenceNonNested"""

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


class MfScSGLevelSequenceNodal(DakotaBaseModel):
    """Generated model for MfScSGLevelSequenceNodal"""

    nodal: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NODAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfScSGLevelSequenceHierarchical(DakotaBaseModel):
    """Generated model for MfScSGLevelSequenceHierarchical"""

    hierarchical: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfScSGLevelSequenceRestricted(DakotaBaseModel):
    """Generated model for MfScSGLevelSequenceRestricted"""

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


class MfScSGLevelSequenceUnrestricted(DakotaBaseModel):
    """Generated model for MfScSGLevelSequenceUnrestricted"""

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


class MfScPiecewise(DakotaBaseModel):
    """Generated model for MfScPiecewise"""

    piecewise: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "METHOD_PIECEWISE",
                    "stored_value": "STD_UNIFORM_U",
                    "secondary_ir_key": "method.nond.piecewise_basis",
                    "secondary_ir_value_type": "bool",
                    "secondary_literal_value": True,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfScAskey(DakotaBaseModel):
    """Generated model for MfScAskey"""

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


class MfScWiener(DakotaBaseModel):
    """Generated model for MfScWiener"""

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


class MfScDiagCov(DakotaBaseModel):
    """Generated model for MfScDiagCov"""

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


class MfScFullCov(DakotaBaseModel):
    """Generated model for MfScFullCov"""

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


class PriorConfig(DakotaBaseModel):
    """Generated model for PriorConfig"""

    multiplier: DakotaFloat = DakotaField(
        default=1.0,
        gt=0.0,
        description="Multiplier to scale prior variance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.prior_prop_cov_mult",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class DerivativesConfig(DakotaBaseModel):
    """Generated model for DerivativesConfig"""

    update_period: int = DakotaField(
        default=INT_MAX,
        description="Period at which to update derivative-based proposal covariance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.prop_cov_update_period",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class ValuesDiag(DakotaBaseModel):
    """Generated model for ValuesDiag"""

    diagonal: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_input_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "diagonal",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ValuesMatrix(DakotaBaseModel):
    """Generated model for ValuesMatrix"""

    matrix: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_input_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "matrix",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ScPRefinementUniform(DakotaBaseModel):
    """Generated model for ScPRefinementUniform"""

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


class ScPRefinementDimAdaptiveSobol(DakotaBaseModel):
    """Generated model for ScPRefinementDimAdaptiveSobol"""

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


class ScPRefinementDimAdaptiveGeneralized(DakotaBaseModel):
    """Generated model for ScPRefinementDimAdaptiveGeneralized"""

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


class BayesScHRefinementLocalAdaptive(DakotaBaseModel):
    """Generated model for BayesScHRefinementLocalAdaptive"""

    local_adaptive: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_control",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_ADAPTIVE_CONTROL",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScQuadratureOrderNested(DakotaBaseModel):
    """Generated model for ScQuadratureOrderNested"""

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


class ScQuadratureOrderNonNested(DakotaBaseModel):
    """Generated model for ScQuadratureOrderNonNested"""

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


class BayesScSGLevelNodal(DakotaBaseModel):
    """Generated model for BayesScSGLevelNodal"""

    nodal: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "NODAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesScSGLevelHierarchical(DakotaBaseModel):
    """Generated model for BayesScSGLevelHierarchical"""

    hierarchical: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesScSGLevelRestricted(DakotaBaseModel):
    """Generated model for BayesScSGLevelRestricted"""

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


class BayesScSGLevelUnrestricted(DakotaBaseModel):
    """Generated model for BayesScSGLevelUnrestricted"""

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


class BayesScPiecewise(DakotaBaseModel):
    """Generated model for BayesScPiecewise"""

    piecewise: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_type",
                    "storage_type": "METHOD_PIECEWISE",
                    "stored_value": "STD_UNIFORM_U",
                    "secondary_ir_key": "method.nond.piecewise_basis",
                    "secondary_ir_value_type": "bool",
                    "secondary_literal_value": True,
                    "ir_value_type": "short",
                }
            ]
        },
    )


class ScAskey(DakotaBaseModel):
    """Generated model for ScAskey"""

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


class ScWiener(DakotaBaseModel):
    """Generated model for ScWiener"""

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


class ScDiagCov(DakotaBaseModel):
    """Generated model for ScDiagCov"""

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


class ScFullCov(DakotaBaseModel):
    """Generated model for ScFullCov"""

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


class BayesSeedRngMt19937(DakotaBaseModel):
    """Generated model for BayesSeedRngMt19937"""

    mt19937: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "mt19937",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BayesSeedRngRnum2(DakotaBaseModel):
    """Generated model for BayesSeedRngRnum2"""

    rnum2: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_number_generator",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "rnum2",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class GPImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for GPImportBuildPointsFileCustomAnnotated"""

    custom_annotated: GPImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=GPImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "GPImportBuildPointsFileCustomAnnotatedConfig",
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


class EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated"""

    custom_annotated: GPImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=GPImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "GPImportBuildPointsFileCustomAnnotatedConfig",
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


class EmulatorPcePRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for EmulatorPcePRefinementDimAdaptive"""

    dimension_adaptive: Union[
        EmulatorPcePRefinementDimAdaptiveSobol,
        EmulatorPcePRefinementDimAdaptiveDecay,
        EmulatorPcePRefinementDimAdaptiveGeneralized,
    ]


class EmulatorMfPcePRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for EmulatorMfPcePRefinementDimAdaptive"""

    dimension_adaptive: Union[
        EmulatorPcePRefinementDimAdaptiveSobol,
        EmulatorPcePRefinementDimAdaptiveDecay,
        EmulatorPcePRefinementDimAdaptiveGeneralized,
    ]


class EmulatorScPRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for EmulatorScPRefinementDimAdaptive"""

    dimension_adaptive: Union[
        EmulatorPcePRefinementDimAdaptiveSobol,
        EmulatorPcePRefinementDimAdaptiveGeneralized,
    ]


class EmulatorPceQuadratureOrderConfig(DakotaBaseModel):
    """Generated model for EmulatorPceQuadratureOrderConfig"""

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
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorMfPceQuadratureOrderSequenceConfig(DakotaBaseModel):
    """Generated model for EmulatorMfPceQuadratureOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpcequadratureordersequenceconfig",
            list_field="sequence",
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
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorScQuadratureOrderConfig(DakotaBaseModel):
    """Generated model for EmulatorScQuadratureOrderConfig"""

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
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorPceSGLevelConfig(DakotaBaseModel):
    """Generated model for EmulatorPceSGLevelConfig"""

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
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorMfPceSGLevelSequenceConfig(DakotaBaseModel):
    """Generated model for EmulatorMfPceSGLevelSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpcesglevelsequenceconfig", list_field="sequence"
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
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorPceExpansionOrderBasisTypeAdapted(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderBasisTypeAdapted"""

    adapted: EmulatorPceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
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


class EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    """Generated model for EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted"""

    adapted: EmulatorPceExpansionOrderBasisTypeAdaptedConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsLeastSquares(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsLeastSquares"""

    least_squares: Union[
        EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd,
        EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon,
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


class EmulatorPceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocRatioLeastSquares"""

    least_squares: Union[
        EmulatorPceExpansionOrderCollocPointsLeastSquaresSvd,
        EmulatorPceExpansionOrderCollocPointsLeastSquaresEqCon,
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


class EmulatorPceExpansionOrderCollocPointsOMP(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsOMP"""

    orthogonal_matching_pursuit: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsBPDN(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsBPDN"""

    basis_pursuit_denoising: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsLars(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsLars"""

    least_angle_regression: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocRatioOMP"""

    orthogonal_matching_pursuit: EmulatorPceExpansionOrderCollocPointsOMPConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocPointsLasso(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsLasso"""

    least_absolute_shrinkage: EmulatorPceExpansionOrderCollocPointsLassoConfig = DakotaField(
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


class EmulatorPceExpansionOrderCollocRatioLasso(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocRatioLasso"""

    least_absolute_shrinkage: EmulatorPceExpansionOrderCollocPointsLassoConfig = DakotaField(
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


class EmulatorPceExpansionOrderExpansionSamples(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderExpansionSamples"""

    expansion_samples: EmulatorPceExpansionOrderExpansionSamplesConfig = DakotaField(
        default=...,
        description="Number of simulation samples used to estimate the expected value of a set of PCE coefficients",
        dakota={"argument": "value"},
    )


class EmulatorMlPceAllocationControlEstimatorVariance(DakotaBaseModel):
    """Generated model for EmulatorMlPceAllocationControlEstimatorVariance"""

    estimator_variance: EmulatorMlPceAllocationControlEstimatorVarianceConfig = (
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


class EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    """Generated model for EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequence"""

    expansion_samples_sequence: EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class EmulatorMfPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    """Generated model for EmulatorMfPceExpansionOrderSequenceExpansionSamplesSequence"""

    expansion_samples_sequence: EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class EmulatorScSGLevelConfig(DakotaBaseModel):
    """Generated model for EmulatorScSGLevelConfig"""

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
    interpolant: Union[EmulatorScSGLevelNodal, EmulatorScSGLevelHierarchical] | None = (
        DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    )
    growth_rule: (
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorMfScSGLevelSequenceConfig(DakotaBaseModel):
    """Generated model for EmulatorMfScSGLevelSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfscsglevelsequenceconfig", list_field="sequence"
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
    interpolant: Union[EmulatorScSGLevelNodal, EmulatorScSGLevelHierarchical] | None = (
        DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    )
    growth_rule: (
        Union[EmulatorPceSGLevelRestricted, EmulatorPceSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[EmulatorPceQuadratureOrderNested, EmulatorPceQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MfScPRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for MfScPRefinementDimAdaptive"""

    dimension_adaptive: Union[
        MfScPRefinementDimAdaptiveSobol, MfScPRefinementDimAdaptiveGeneralized
    ]


class MfScHRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for MfScHRefinementDimAdaptive"""

    dimension_adaptive: Union[
        MfScPRefinementDimAdaptiveSobol, MfScPRefinementDimAdaptiveGeneralized
    ]


class MfScQuadratureOrderSequenceConfig(DakotaBaseModel):
    """Generated model for MfScQuadratureOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfscquadratureordersequenceconfig", list_field="sequence"
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
        Union[MfScQuadratureOrderSequenceNested, MfScQuadratureOrderSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfScSGLevelSequenceConfig(DakotaBaseModel):
    """Generated model for BayesMfScSGLevelSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesmfscsglevelsequenceconfig", list_field="sequence"
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
    interpolant: (
        Union[MfScSGLevelSequenceNodal, MfScSGLevelSequenceHierarchical] | None
    ) = DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    growth_rule: (
        Union[MfScSGLevelSequenceRestricted, MfScSGLevelSequenceUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[MfScQuadratureOrderSequenceNested, MfScQuadratureOrderSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class Prior(DakotaBaseModel):
    """Generated model for Prior"""

    prior: PriorConfig = DakotaField(
        description="Uses the covariance of the prior distributions to define the MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "prior",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Derivatives(DakotaBaseModel):
    """Generated model for Derivatives"""

    derivatives: DerivativesConfig = DakotaField(
        description="Use derivatives to inform the MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "derivatives",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ValuesConfig(DakotaBaseModel):
    """Generated model for ValuesConfig"""

    elements: list[DakotaFloat] = DakotaField(
        description="Specifies matrix values to use as the MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_data",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    covariance_form: Union[ValuesDiag, ValuesMatrix] = DakotaField(
        description="Values For", dakota={"anchor": True, "union_pattern": 4}
    )


class FileConfig(DakotaBaseModel):
    """Generated model for FileConfig"""

    filename: str = DakotaField(
        description="Uses a file to import a user-specified MCMC proposal covariance.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.proposal_covariance_filename",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    covariance_form: Union[ValuesDiag, ValuesMatrix] = DakotaField(
        description="Values For", dakota={"anchor": True, "union_pattern": 4}
    )


class BayesScPRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for BayesScPRefinementDimAdaptive"""

    dimension_adaptive: Union[
        ScPRefinementDimAdaptiveSobol, ScPRefinementDimAdaptiveGeneralized
    ]


class ScHRefinementDimAdaptive(DakotaBaseModel):
    """Generated model for ScHRefinementDimAdaptive"""

    dimension_adaptive: Union[
        ScPRefinementDimAdaptiveSobol, ScPRefinementDimAdaptiveGeneralized
    ]


class BayesScQuadratureOrderConfig(DakotaBaseModel):
    """Generated model for BayesScQuadratureOrderConfig"""

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
    nesting_rule: Union[ScQuadratureOrderNested, ScQuadratureOrderNonNested] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Nesting",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class BayesScSGLevelConfig(DakotaBaseModel):
    """Generated model for BayesScSGLevelConfig"""

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
    interpolant: Union[BayesScSGLevelNodal, BayesScSGLevelHierarchical] | None = (
        DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    )
    growth_rule: Union[BayesScSGLevelRestricted, BayesScSGLevelUnrestricted] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Growth",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )
    nesting_rule: Union[ScQuadratureOrderNested, ScQuadratureOrderNonNested] | None = (
        DakotaField(
            default=None,
            description="Quadrature Rule Nesting",
            dakota={"anchor": True, "union_pattern": 2},
        )
    )


class BayesSeedRngMixin(DakotaBaseModel):
    """Generated model for BayesSeedRngMixin"""

    seed: int | None = DakotaField(
        default=None,
        gt=0,
        description="Seed of the random number generator",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_seed",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    rng: Union[BayesSeedRngMt19937, BayesSeedRngRnum2] = DakotaField(
        default_factory=BayesSeedRngMt19937,
        description="Selection of a random number generator",
        dakota={"union_pattern": 1, "model_default": "BayesSeedRngMt19937"},
    )


class GPImportBuildPointsFile(DakotaBaseModel):
    """Generated model for GPImportBuildPointsFile"""

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
        GPImportBuildPointsFileCustomAnnotated,
        GPImportBuildPointsFileAnnotated,
        GPImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GPImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GPImportBuildPointsFileAnnotated",
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


class EmulatorPceExpansionOrderImportBuildPointsFile(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderImportBuildPointsFile"""

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
        EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated,
        GPImportBuildPointsFileAnnotated,
        GPImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GPImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GPImportBuildPointsFileAnnotated",
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


class EmulatorPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    """Generated model for EmulatorPceOrthogLeastInterpImportBuildPointsFile"""

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
        EmulatorPceExpansionOrderImportBuildPointsFileCustomAnnotated,
        GPImportBuildPointsFileAnnotated,
        GPImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=GPImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "GPImportBuildPointsFileAnnotated",
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


class EmulatorScPRefinement(DakotaBaseModel):
    """Generated model for EmulatorScPRefinement"""

    p_refinement: Union[
        EmulatorPcePRefinementUniform, EmulatorScPRefinementDimAdaptive
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class EmulatorScHRefinement(DakotaBaseModel):
    """Generated model for EmulatorScHRefinement"""

    h_refinement: Union[
        EmulatorPcePRefinementUniform,
        EmulatorScPRefinementDimAdaptive,
        EmulatorScHRefinementLocalAdaptive,
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "H_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class EmulatorMfScPRefinement(DakotaBaseModel):
    """Generated model for EmulatorMfScPRefinement"""

    p_refinement: Union[
        EmulatorPcePRefinementUniform, EmulatorScPRefinementDimAdaptive
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "P_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class EmulatorMfScHRefinement(DakotaBaseModel):
    """Generated model for EmulatorMfScHRefinement"""

    h_refinement: Union[
        EmulatorPcePRefinementUniform,
        EmulatorScPRefinementDimAdaptive,
        EmulatorScHRefinementLocalAdaptive,
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "H_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class EmulatorPceQuadratureOrder(DakotaBaseModel):
    """Generated model for EmulatorPceQuadratureOrder"""

    quadrature_order: EmulatorPceQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class EmulatorMfPceQuadratureOrderSequence(DakotaBaseModel):
    """Generated model for EmulatorMfPceQuadratureOrderSequence"""

    quadrature_order_sequence: EmulatorMfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorMfScQuadratureOrderSequence(DakotaBaseModel):
    """Generated model for EmulatorMfScQuadratureOrderSequence"""

    quadrature_order_sequence: EmulatorMfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorScQuadratureOrder(DakotaBaseModel):
    """Generated model for EmulatorScQuadratureOrder"""

    quadrature_order: EmulatorScQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class EmulatorPceSGLevel(DakotaBaseModel):
    """Generated model for EmulatorPceSGLevel"""

    sparse_grid_level: EmulatorPceSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class EmulatorMfPceSGLevelSequence(DakotaBaseModel):
    """Generated model for EmulatorMfPceSGLevelSequence"""

    sparse_grid_level_sequence: EmulatorMfPceSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorPceExpansionOrderCollocPointsConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPointsConfig"""

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
            EmulatorPceExpansionOrderCollocPointsLeastSquares,
            EmulatorPceExpansionOrderCollocPointsOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocPointsLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorPceExpansionOrderCollocRatioConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocRatioConfig"""

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
            EmulatorPceExpansionOrderCollocRatioLeastSquares,
            EmulatorPceExpansionOrderCollocRatioOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorMlPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    """Generated model for EmulatorMlPceExpansionOrderSequenceCollocRatioConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceexpansionordersequencecollocratioconfig",
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
            EmulatorPceExpansionOrderCollocRatioLeastSquares,
            EmulatorPceExpansionOrderCollocRatioOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorMfPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    """Generated model for EmulatorMfPceExpansionOrderSequenceCollocRatioConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpceexpansionordersequencecollocratioconfig",
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
            EmulatorPceExpansionOrderCollocRatioLeastSquares,
            EmulatorPceExpansionOrderCollocRatioOMP,
            EmulatorPceExpansionOrderCollocPointsBP,
            EmulatorPceExpansionOrderCollocPointsBPDN,
            EmulatorPceExpansionOrderCollocPointsLars,
            EmulatorPceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: EmulatorPceExpansionOrderCollocPointsCV | None = DakotaField(
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


class EmulatorScSGLevel(DakotaBaseModel):
    """Generated model for EmulatorScSGLevel"""

    sparse_grid_level: EmulatorScSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class EmulatorMfScSGLevelSequence(DakotaBaseModel):
    """Generated model for EmulatorMfScSGLevelSequence"""

    sparse_grid_level_sequence: EmulatorMfScSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfScPRefinement(DakotaBaseModel):
    """Generated model for BayesMfScPRefinement"""

    p_refinement: Union[MfScPRefinementUniform, MfScPRefinementDimAdaptive] = (
        DakotaField(
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.expansion_refinement_type",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "P_REFINEMENT",
                        "enum_scope": "Pecos",
                        "ir_value_type": "short",
                    }
                ]
            }
        )
    )


class BayesMfScHRefinement(DakotaBaseModel):
    """Generated model for BayesMfScHRefinement"""

    h_refinement: Union[
        MfScPRefinementUniform, MfScHRefinementDimAdaptive, MfScHRefinementLocalAdaptive
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "H_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class BayesMfScQuadratureOrderSequence(DakotaBaseModel):
    """Generated model for BayesMfScQuadratureOrderSequence"""

    quadrature_order_sequence: MfScQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesMfScSGLevelSequence(DakotaBaseModel):
    """Generated model for BayesMfScSGLevelSequence"""

    sparse_grid_level_sequence: BayesMfScSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class Values(DakotaBaseModel):
    """Generated model for Values"""

    values: ValuesConfig = DakotaField(
        default=...,
        description="Specifies matrix values to use as the MCMC proposal covariance.",
        dakota={"argument": "elements"},
    )


class ProposalCovFile(DakotaBaseModel):
    """Generated model for ProposalCovFile"""

    file: FileConfig = DakotaField(
        default=...,
        description="Uses a file to import a user-specified MCMC proposal covariance.",
        dakota={"argument": "filename"},
    )


class BayesScPRefinement(DakotaBaseModel):
    """Generated model for BayesScPRefinement"""

    p_refinement: Union[ScPRefinementUniform, BayesScPRefinementDimAdaptive] = (
        DakotaField(
            dakota={
                "materialization": [
                    {
                        "ir_key": "method.nond.expansion_refinement_type",
                        "storage_type": "PRESENCE_ENUM",
                        "stored_value": "P_REFINEMENT",
                        "enum_scope": "Pecos",
                        "ir_value_type": "short",
                    }
                ]
            }
        )
    )


class BayesScHRefinement(DakotaBaseModel):
    """Generated model for BayesScHRefinement"""

    h_refinement: Union[
        ScPRefinementUniform, ScHRefinementDimAdaptive, BayesScHRefinementLocalAdaptive
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_refinement_type",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "H_REFINEMENT",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class BayesScQuadratureOrder(DakotaBaseModel):
    """Generated model for BayesScQuadratureOrder"""

    quadrature_order: BayesScQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class BayesScSGLevel(DakotaBaseModel):
    """Generated model for BayesScSGLevel"""

    sparse_grid_level: BayesScSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class EmulatorGPConfig(DakotaBaseModel):
    """Generated model for EmulatorGPConfig"""

    implementation: Union[EmulatorGPSurfpack, EmulatorGPDakota] = DakotaField(
        description="GP Implementation", dakota={"anchor": True, "union_pattern": 4}
    )
    build_samples: int = DakotaField(
        default=0,
        description="Number of initial model evaluations used in build phase",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.build_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
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
    import_build_points_file: GPImportBuildPointsFile | None = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorPceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for EmulatorPceOrthogLeastInterpConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatorpceorthogleastinterpconfig", list_field="tensor_grid"
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
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
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


class EmulatorMlPceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for EmulatorMlPceOrthogLeastInterpConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceorthogleastinterpconfig",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="bayesemulatormlpceorthogleastinterpconfig",
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
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorPceExpansionOrderCollocPoints(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocPoints"""

    collocation_points: EmulatorPceExpansionOrderCollocPointsConfig = DakotaField(
        default=...,
        description="Number of collocation points used to estimate expansion coefficients",
        dakota={"argument": "points"},
    )


class EmulatorPceExpansionOrderCollocRatio(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderCollocRatio"""

    collocation_ratio: EmulatorPceExpansionOrderCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "value"},
    )


class EmulatorMlPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    """Generated model for EmulatorMlPceExpansionOrderSequenceCollocRatio"""

    collocation_ratio: EmulatorMlPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class EmulatorMfPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    """Generated model for EmulatorMfPceExpansionOrderSequenceCollocRatio"""

    collocation_ratio: EmulatorMfPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class ScConfig(DakotaBaseModel):
    """Generated model for ScConfig"""

    refinement_method: Union[EmulatorScPRefinement, EmulatorScHRefinement] | None = (
        DakotaField(
            default=None,
            description="Automated Refinement Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
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
    grid_type: Union[EmulatorScQuadratureOrder, EmulatorScSGLevel] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )
    basis_family: (
        Union[EmulatorScPiecewise, EmulatorPceAskey, EmulatorPceWiener] | None
    ) = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MfScConfig(DakotaBaseModel):
    """Generated model for MfScConfig"""

    refinement_method: (
        Union[EmulatorMfScPRefinement, EmulatorMfScHRefinement] | None
    ) = DakotaField(
        default=None,
        description="Automated Refinement Type",
        dakota={"anchor": True, "union_pattern": 2},
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
    allocation_control: Union[EmulatorMfPceAllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )
    discrepancy_emulation: (
        Union[
            EmulatorMlPceDiscrepancyEmulationDistinct,
            EmulatorMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    grid_type: Union[
        EmulatorMfScQuadratureOrderSequence, EmulatorMfScSGLevelSequence
    ] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )
    basis_family: (
        Union[EmulatorScPiecewise, EmulatorPceAskey, EmulatorPceWiener] | None
    ) = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesMfScMfSc(DakotaBaseModel):
    """Generated model for BayesMfScMfSc"""

    refinement_method: Union[BayesMfScPRefinement, BayesMfScHRefinement] | None = (
        DakotaField(
            default=None,
            description="Automated Refinement Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
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
    allocation_control: Union[MfScAllocationControlGreedy] | None = DakotaField(
        default=None,
        description="Sample allocation approach for multifidelity expansions",
        dakota={"union_pattern": 2},
    )
    discrepancy_emulation: (
        Union[MfScDiscrepancyEmulationDistinct, MfScDiscrepancyEmulationRecursive]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    grid_type: Union[BayesMfScQuadratureOrderSequence, BayesMfScSGLevelSequence] = (
        DakotaField(
            description="Interpolation Grid Type",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )
    basis_family: Union[MfScPiecewise, MfScAskey, MfScWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[MfScDiagCov, MfScFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesProposalCovMixin(DakotaBaseModel):
    """Generated model for BayesProposalCovMixin"""

    proposal_covariance: Union[Prior, Derivatives, Values, ProposalCovFile] | None = (
        DakotaField(
            default=None,
            description="Defines the technique used to generate the MCMC proposal covariance.",
            dakota={
                "union_pattern": 2,
                "materialization": [
                    {
                        "ir_key": "method.nond.proposal_covariance_type",
                        "storage_type": "PRESENCE_LITERAL",
                        "stored_value": "user",
                        "ir_value_type": "String",
                    }
                ],
            },
        )
    )


class BayesScSc(DakotaBaseModel):
    """Generated model for BayesScSc"""

    refinement_method: Union[BayesScPRefinement, BayesScHRefinement] | None = (
        DakotaField(
            default=None,
            description="Automated Refinement Type",
            dakota={"anchor": True, "union_pattern": 2},
        )
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
    grid_type: Union[BayesScQuadratureOrder, BayesScSGLevel] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )
    basis_family: Union[BayesScPiecewise, ScAskey, ScWiener] | None = DakotaField(
        default=None,
        description="Basis Polynomial Family",
        dakota={"anchor": True, "union_pattern": 2},
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
    covariance_type: Union[ScDiagCov, ScFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorGP(DakotaBaseModel):
    """Generated model for EmulatorGP"""

    gaussian_process: EmulatorGPConfig = DakotaField(
        description="Gaussian Process surrogate model", dakota={"aliases": ["kriging"]}
    )


class EmulatorPceOrthogLeastInterp(DakotaBaseModel):
    """Generated model for EmulatorPceOrthogLeastInterp"""

    orthogonal_least_interpolation: EmulatorPceOrthogLeastInterpConfig = DakotaField(
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


class EmulatorMlPceOrthogLeastInterp(DakotaBaseModel):
    """Generated model for EmulatorMlPceOrthogLeastInterp"""

    orthogonal_least_interpolation: EmulatorMlPceOrthogLeastInterpConfig = DakotaField(
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


class EmulatorMfPceOrthogLeastInterp(DakotaBaseModel):
    """Generated model for EmulatorMfPceOrthogLeastInterp"""

    orthogonal_least_interpolation: EmulatorMlPceOrthogLeastInterpConfig = DakotaField(
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


class EmulatorPceExpansionOrderConfig(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrderConfig"""

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
            EmulatorPceExpansionOrderBasisTypeTensorProduct,
            EmulatorPceExpansionOrderBasisTypeTotalOrder,
            EmulatorPceExpansionOrderBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_selection: Union[
        EmulatorPceExpansionOrderCollocPoints,
        EmulatorPceExpansionOrderCollocRatio,
        EmulatorPceExpansionOrderExpansionSamples,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: EmulatorPceExpansionOrderImportBuildPointsFile | None = (
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


class EmulatorMlPceExpansionOrderSequenceConfig(DakotaBaseModel):
    """Generated model for EmulatorMlPceExpansionOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormlpceexpansionordersequenceconfig",
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
            EmulatorPceExpansionOrderBasisTypeTensorProduct,
            EmulatorPceExpansionOrderBasisTypeTotalOrder,
            EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        EmulatorMlPceExpansionOrderSequenceCollocRatio,
        EmulatorMlPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorMfPceExpansionOrderSequenceConfig(DakotaBaseModel):
    """Generated model for EmulatorMfPceExpansionOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="bayesemulatormfpceexpansionordersequenceconfig",
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
            EmulatorPceExpansionOrderBasisTypeTensorProduct,
            EmulatorPceExpansionOrderBasisTypeTotalOrder,
            EmulatorMlPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        EmulatorMfPceExpansionOrderSequenceCollocRatio,
        EmulatorMfPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        EmulatorPceOrthogLeastInterpImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class EmulatorSc(DakotaBaseModel):
    """Generated model for EmulatorSc"""

    sc: ScConfig = DakotaField(
        description="Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorMfSc(DakotaBaseModel):
    """Generated model for EmulatorMfSc"""

    mf_sc: MfScConfig = DakotaField(
        description="Multifidelity Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MF_SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesMfScMixin(DakotaBaseModel):
    """Generated model for BayesMfScMixin"""

    mf_sc: BayesMfScMfSc = DakotaField(
        description="Multifidelity Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MF_SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class BayesScMixin(DakotaBaseModel):
    """Generated model for BayesScMixin"""

    sc: BayesScSc = DakotaField(
        description="Stochastic Collocation as an emulator model.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.emulator",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SC_EMULATOR",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class EmulatorPceExpansionOrder(DakotaBaseModel):
    """Generated model for EmulatorPceExpansionOrder"""

    expansion_order: EmulatorPceExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class EmulatorMlPceExpansionOrderSequence(DakotaBaseModel):
    """Generated model for EmulatorMlPceExpansionOrderSequence"""

    expansion_order_sequence: EmulatorMlPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class EmulatorMfPceExpansionOrderSequence(DakotaBaseModel):
    """Generated model for EmulatorMfPceExpansionOrderSequence"""

    expansion_order_sequence: EmulatorMfPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class BayesEmulatorPceConfig(DakotaBaseModel):
    """Generated model for BayesEmulatorPceConfig"""

    p_refinement: (
        Union[EmulatorPcePRefinementUniform, EmulatorPcePRefinementDimAdaptive] | None
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
        EmulatorPceQuadratureOrder,
        EmulatorPceSGLevel,
        EmulatorPceCubatureIntegrand,
        EmulatorPceExpansionOrder,
        EmulatorPceOrthogLeastInterp,
        EmulatorPceImportExpansionFile,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[EmulatorPceAskey, EmulatorPceWiener] | None = DakotaField(
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesEmulatorMlPceConfig(DakotaBaseModel):
    """Generated model for BayesEmulatorMlPceConfig"""

    allocation_control: (
        Union[
            EmulatorMlPceAllocationControlEstimatorVariance,
            EmulatorMlPceAllocationControlRipSampling,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Sample allocation approach for multilevel expansions",
        dakota={"union_pattern": 2},
    )
    discrepancy_emulation: (
        Union[
            EmulatorMlPceDiscrepancyEmulationDistinct,
            EmulatorMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    coefficient_approach: Union[
        EmulatorMlPceExpansionOrderSequence, EmulatorMlPceOrthogLeastInterp
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[EmulatorPceAskey, EmulatorPceWiener] | None = DakotaField(
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class BayesEmulatorMfPceConfig(DakotaBaseModel):
    """Generated model for BayesEmulatorMfPceConfig"""

    p_refinement: (
        Union[EmulatorPcePRefinementUniform, EmulatorMfPcePRefinementDimAdaptive] | None
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
    allocation_control: Union[EmulatorMfPceAllocationControlGreedy] | None = (
        DakotaField(
            default=None,
            description="Sample allocation approach for multifidelity expansions",
            dakota={"union_pattern": 2},
        )
    )
    discrepancy_emulation: (
        Union[
            EmulatorMlPceDiscrepancyEmulationDistinct,
            EmulatorMlPceDiscrepancyEmulationRecursive,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Formulation for emulation of model discrepancies.",
        dakota={"union_pattern": 2},
    )
    coefficient_approach: Union[
        EmulatorMfPceQuadratureOrderSequence,
        EmulatorMfPceSGLevelSequence,
        EmulatorMfPceExpansionOrderSequence,
        EmulatorMfPceOrthogLeastInterp,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    basis_family: Union[EmulatorPceAskey, EmulatorPceWiener] | None = DakotaField(
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
    covariance_type: Union[EmulatorPceDiagCov, EmulatorPceFullCov] | None = DakotaField(
        default=None,
        description="Covariance Type",
        dakota={"anchor": True, "union_pattern": 2},
    )


class EmulatorPce(DakotaBaseModel):
    """Generated model for EmulatorPce"""

    pce: BayesEmulatorPceConfig = DakotaField(
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


class EmulatorMlPce(DakotaBaseModel):
    """Generated model for EmulatorMlPce"""

    ml_pce: BayesEmulatorMlPceConfig = DakotaField(
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


class EmulatorMfPce(DakotaBaseModel):
    """Generated model for EmulatorMfPce"""

    mf_pce: BayesEmulatorMfPceConfig = DakotaField(
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


class BayesEmulatorMixin(DakotaBaseModel):
    """Generated model for BayesEmulatorMixin"""

    emulator: (
        Union[
            EmulatorGP,
            EmulatorPce,
            EmulatorMlPce,
            EmulatorMfPce,
            EmulatorSc,
            EmulatorMfSc,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Use an emulator or surrogate model to evaluate the likelihood function",
        dakota={"union_pattern": 2},
    )
