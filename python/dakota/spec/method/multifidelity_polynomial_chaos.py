"""Generated Pydantic models for method.multifidelity_polynomial_chaos"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from ..base import SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ..validation import ValidationRule
from ..validation.rules import CheckNonnegativeList

# Cross-module model imports
from dakota.spec.shared.core import MethodMfExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.expansion.pce import PceOptionsMixin, PceRefinementMixin
from dakota.spec.shared.misc import (
    DiscrepEmulationMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MfAllocControlContext2Mixin,
)


class MfPceQuadratureOrderSequenceNested(DakotaBaseModel):
    """Generated model for MfPceQuadratureOrderSequenceNested"""

    nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceQuadratureOrderSequenceNonNested(DakotaBaseModel):
    """Generated model for MfPceQuadratureOrderSequenceNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NON_NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceSGLevelSequenceRestricted(DakotaBaseModel):
    """Generated model for MfPceSGLevelSequenceRestricted"""

    restricted: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "stored_value": "RESTRICTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceSGLevelSequenceUnrestricted(DakotaBaseModel):
    """Generated model for MfPceSGLevelSequenceUnrestricted"""

    unrestricted: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.growth_override",
                    "stored_value": "UNRESTRICTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceSGLevelSequenceNested(DakotaBaseModel):
    """Generated model for MfPceSGLevelSequenceNested"""

    nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceSGLevelSequenceNonNested(DakotaBaseModel):
    """Generated model for MfPceSGLevelSequenceNonNested"""

    non_nested: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.nesting_override",
                    "stored_value": "NON_NESTED",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceBasisTypeTensorProduct(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceBasisTypeTensorProduct"""

    tensor_product: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "stored_value": "TENSOR_PRODUCT_BASIS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceBasisTypeTotalOrder(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceBasisTypeTotalOrder"""

    total_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "stored_value": "TOTAL_ORDER_BASIS",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceBasisTypeAdaptedConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceBasisTypeAdaptedConfig"""

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


class MfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd"""

    svd: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "stored_value": "SVD_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon"""

    equality_constrained: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.least_squares_regression_type",
                    "stored_value": "EQ_CON_LS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceCollocRatioOMPConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioOMPConfig"""

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


class MfPceExpansionOrderSequenceCollocRatioBP(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioBP"""

    basis_pursuit: Literal[True] = DakotaField(
        default=True,
        dakota={
            "aliases": ["bp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "BASIS_PURSUIT",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceExpansionOrderSequenceCollocRatioBPDNConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioBPDNConfig"""

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


class MfPceExpansionOrderSequenceCollocRatioLarsConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLarsConfig"""

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


class MfPceExpansionOrderSequenceCollocRatioLassoConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLassoConfig"""

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


class MfPceExpansionOrderSequenceCollocRatioCV(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioCV"""

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


class MfPceExpansionOrderSequenceExpansionSamplesSequenceConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceExpansionSamplesSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mfpceexpansionordersequenceexpansionsamplessequenceconfig",
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
                    "stored_value": "all",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig(
    DakotaBaseModel
):
    """Generated model for MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_HEADER",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_EVAL_ID",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceImportBuildPointsFileAnnotated(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceImportBuildPointsFileFreeform(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig"""

    header: Literal[True] | None = DakotaField(
        default=None,
        description="Enable header row in custom-annotated tabular file",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_HEADER",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_EVAL_ID",
                    "storage_type": "AUGMENT_ENUM",
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
                    "stored_value": "TABULAR_IFACE_ID",
                    "storage_type": "AUGMENT_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceOrthogLeastInterpImportBuildPointsFileAnnotated(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterpImportBuildPointsFileAnnotated"""

    annotated: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_ANNOTATED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceOrthogLeastInterpImportBuildPointsFileFreeform(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterpImportBuildPointsFileFreeform"""

    freeform: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class MfPceQuadratureOrderSequenceConfig(DakotaBaseModel):
    """Generated model for MfPceQuadratureOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mfpcequadratureordersequenceconfig", list_field="sequence"
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
        Union[MfPceQuadratureOrderSequenceNested, MfPceQuadratureOrderSequenceNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MfPceSGLevelSequenceConfig(DakotaBaseModel):
    """Generated model for MfPceSGLevelSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mfpcesglevelsequenceconfig", list_field="sequence"
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
        Union[MfPceSGLevelSequenceRestricted, MfPceSGLevelSequenceUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[MfPceSGLevelSequenceNested, MfPceSGLevelSequenceNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MfPceExpansionOrderSequenceBasisTypeAdapted(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceBasisTypeAdapted"""

    adapted: MfPceExpansionOrderSequenceBasisTypeAdaptedConfig = DakotaField(
        description="Use adaptive basis selection to choose the basis terms in a polynomial chaos expansion.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "stored_value": "ADAPTED_BASIS_EXPANDING_FRONT",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MfPceExpansionOrderSequenceCollocRatioLeastSquares(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLeastSquares"""

    least_squares: Union[
        MfPceExpansionOrderSequenceCollocRatioLeastSquaresSvd,
        MfPceExpansionOrderSequenceCollocRatioLeastSquaresEqCon,
        dict,
    ] = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "DEFAULT_LEAST_SQ_REGRESSION",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        }
    )


class MfPceExpansionOrderSequenceCollocRatioOMP(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioOMP"""

    orthogonal_matching_pursuit: MfPceExpansionOrderSequenceCollocRatioOMPConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion using orthogonal matching pursuit (OMP)",
        dakota={
            "aliases": ["omp"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "ORTHOG_MATCH_PURSUIT",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceExpansionOrderSequenceCollocRatioBPDN(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioBPDN"""

    basis_pursuit_denoising: MfPceExpansionOrderSequenceCollocRatioBPDNConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by solving the Basis Pursuit Denoising L1 -minimization problem using second order cone optimization.",
        dakota={
            "aliases": ["bpdn"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "BASIS_PURSUIT_DENOISING",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceExpansionOrderSequenceCollocRatioLars(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLars"""

    least_angle_regression: MfPceExpansionOrderSequenceCollocRatioLarsConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the greedy least angle regression (LAR) method.",
        dakota={
            "aliases": ["lars"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "LEAST_ANGLE_REGRESSION",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceExpansionOrderSequenceCollocRatioLasso(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioLasso"""

    least_absolute_shrinkage: MfPceExpansionOrderSequenceCollocRatioLassoConfig = DakotaField(
        description="Compute the coefficients of a polynomial expansion by using the LASSO problem.",
        dakota={
            "aliases": ["lasso"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "LASSO_REGRESSION",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceExpansionOrderSequenceExpansionSamplesSequence(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceExpansionSamplesSequence"""

    expansion_samples_sequence: MfPceExpansionOrderSequenceExpansionSamplesSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion samples used in a multi-stage polynomial chaos expansion Each level entry of the ``expansion_samples_sequence`` applies to one expansion within a multi-stage expansion.  Current multi-stage expansions that support expansion samples sequences include multilevel and multifidelity polynomial chaos. If adaptive refinement is active, then this sequence specifies the starting point for each level within either an individual or integrated refinement approach. A corresponding scalar specification is documented at, e.g., :dakkw:`method-polynomial_chaos-expansion_order-expansion_samples`",
        dakota={"argument": "sequence"},
    )


class MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated"""

    custom_annotated: MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

    custom_annotated: MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
            "materialization": [
                {
                    "ir_key": "method.import_build_format",
                    "stored_value": "TABULAR_NONE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class MfPceQuadratureOrderSequence(DakotaBaseModel):
    """Generated model for MfPceQuadratureOrderSequence"""

    quadrature_order_sequence: MfPceQuadratureOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of quadrature orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class MfPceSGLevelSequence(DakotaBaseModel):
    """Generated model for MfPceSGLevelSequence"""

    sparse_grid_level_sequence: MfPceSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class MfPceExpansionOrderSequenceCollocRatioConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatioConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mfpceexpansionordersequencecollocratioconfig",
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
            MfPceExpansionOrderSequenceCollocRatioLeastSquares,
            MfPceExpansionOrderSequenceCollocRatioOMP,
            MfPceExpansionOrderSequenceCollocRatioBP,
            MfPceExpansionOrderSequenceCollocRatioBPDN,
            MfPceExpansionOrderSequenceCollocRatioLars,
            MfPceExpansionOrderSequenceCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: MfPceExpansionOrderSequenceCollocRatioCV | None = DakotaField(
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
                    "stored_value": "all",
                    "storage_type": "PRESENCE_LITERAL",
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


class MfPceExpansionOrderSequenceImportBuildPointsFile(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceImportBuildPointsFile"""

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
        MfPceExpansionOrderSequenceImportBuildPointsFileCustomAnnotated,
        MfPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        MfPceExpansionOrderSequenceImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MfPceExpansionOrderSequenceImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MfPceExpansionOrderSequenceImportBuildPointsFileAnnotated",
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


class MfPceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterpImportBuildPointsFile"""

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
        MfPceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        MfPceOrthogLeastInterpImportBuildPointsFileAnnotated,
        MfPceOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=MfPceOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "MfPceOrthogLeastInterpImportBuildPointsFileAnnotated",
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


class MfPceExpansionOrderSequenceCollocRatio(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceCollocRatio"""

    collocation_ratio: MfPceExpansionOrderSequenceCollocRatioConfig = DakotaField(
        default=...,
        description="Set the number of points used to build a PCE via regression to be proportional to the number of terms in the expansion.",
        dakota={"argument": "sequence"},
    )


class MfPceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterpConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mfpceorthogleastinterpconfig",
            list_field="collocation_points_sequence",
        ),
        CheckNonnegativeList(
            context="mfpceorthogleastinterpconfig", list_field="tensor_grid"
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
                    "stored_value": "all",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    import_build_points_file: MfPceOrthogLeastInterpImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )


class MfPceExpansionOrderSequenceConfig(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="mfpceexpansionordersequenceconfig", list_field="sequence"
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
            MfPceExpansionOrderSequenceBasisTypeTensorProduct,
            MfPceExpansionOrderSequenceBasisTypeTotalOrder,
            MfPceExpansionOrderSequenceBasisTypeAdapted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Specify the type of basis truncation to be used for a Polynomial Chaos Expansion.",
        dakota={"union_pattern": 2},
    )
    point_sequence_selection: Union[
        MfPceExpansionOrderSequenceCollocRatio,
        MfPceExpansionOrderSequenceExpansionSamplesSequence,
    ] = DakotaField(dakota={"anchor": True, "union_pattern": 4})
    import_build_points_file: (
        MfPceExpansionOrderSequenceImportBuildPointsFile | None
    ) = DakotaField(
        default=None,
        description="File containing points you wish to use to build a surrogate",
        dakota={"argument": "filename", "aliases": ["import_points_file"]},
    )


class MfPceOrthogLeastInterp(DakotaBaseModel):
    """Generated model for MfPceOrthogLeastInterp"""

    orthogonal_least_interpolation: MfPceOrthogLeastInterpConfig = DakotaField(
        description="Build a polynomial chaos expansion from simulation samples using orthogonal least interpolation.",
        dakota={
            "aliases": ["least_interpolation", "oli"],
            "materialization": [
                {
                    "ir_key": "method.nond.regression_type",
                    "stored_value": "ORTHOG_LEAST_INTERPOLATION",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ],
        },
    )


class MfPceExpansionOrderSequence(DakotaBaseModel):
    """Generated model for MfPceExpansionOrderSequence"""

    expansion_order_sequence: MfPceExpansionOrderSequenceConfig = DakotaField(
        default=...,
        description="Sequence of expansion orders used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class MfPceConfig(
    MethodThreeOptionalKeywordsMixin,
    PceRefinementMixin,
    MethodMfExpConvergenceToleranceMixin,
    MfAllocControlContext2Mixin,
    DiscrepEmulationMixin,
    PceOptionsMixin,
    ExpansionOptionsMixin,
    MethodSeedSequenceMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for MfPceConfig"""

    coefficient_approach: Union[
        MfPceQuadratureOrderSequence,
        MfPceSGLevelSequence,
        MfPceExpansionOrderSequence,
        MfPceOrthogLeastInterp,
    ] = DakotaField(
        description="Chaos Coefficient Estimation Approach",
        dakota={"anchor": True, "union_pattern": 4},
    )


class MfPceSelection(MethodSelection):
    """Generated model for MfPceSelection"""

    multifidelity_polynomial_chaos: MfPceConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "MULTIFIDELITY_POLYNOMIAL_CHAOS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
