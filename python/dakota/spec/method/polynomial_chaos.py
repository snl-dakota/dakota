"""Generated Pydantic models for method.polynomial_chaos"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from ..base import SZ_MAX, USHRT_MAX
from typing import ClassVar, List
from ..validation import ValidationRule
from ..validation.rules import CheckNonnegativeList

# Cross-module model imports
from dakota.spec.shared.core import MethodExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.expansion.pce import PceOptionsMixin, PceRefinementMixin
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
)


class PceQuadratureOrderNested(DakotaBaseModel):
    """Generated model for PceQuadratureOrderNested"""

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


class PceQuadratureOrderNonNested(DakotaBaseModel):
    """Generated model for PceQuadratureOrderNonNested"""

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


class PceCubatureIntegrand(DakotaBaseModel):
    """Generated model for PceCubatureIntegrand"""

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


class ExpansionOrderCollocPointsBPDNConfig(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocPointsBPDNConfig"""

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


class ExpansionOrderCollocPointsLarsConfig(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocPointsLarsConfig"""

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


class ExpansionOrderCollocRatioLeastSquaresSvd(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioLeastSquaresSvd"""

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


class ExpansionOrderCollocRatioLeastSquaresEqCon(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioLeastSquaresEqCon"""

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


class ExpansionOrderCollocRatioOMPConfig(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioOMPConfig"""

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


class ExpansionOrderCollocRatioBP(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioBP"""

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


class ExpansionOrderCollocRatioBPDNConfig(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioBPDNConfig"""

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


class ExpansionOrderCollocRatioLarsConfig(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioLarsConfig"""

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


class ExpansionOrderCollocRatioLassoConfig(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioLassoConfig"""

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


class ExpansionOrderCollocRatioCV(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioCV"""

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


class PceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig"""

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


class PceOrthogLeastInterpImportBuildPointsFileAnnotated(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpImportBuildPointsFileAnnotated"""

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


class PceOrthogLeastInterpImportBuildPointsFileFreeform(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpImportBuildPointsFileFreeform"""

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


class PceImportExpansionFile(DakotaBaseModel):
    """Generated model for PceImportExpansionFile"""

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


class PceQuadratureOrderConfig(DakotaBaseModel):
    """Generated model for PceQuadratureOrderConfig"""

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
        Union[PceQuadratureOrderNested, PceQuadratureOrderNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class PceSGLevelConfig(DakotaBaseModel):
    """Generated model for PceSGLevelConfig"""

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

    basis_pursuit_denoising: ExpansionOrderCollocPointsBPDNConfig = DakotaField(
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

    least_angle_regression: ExpansionOrderCollocPointsLarsConfig = DakotaField(
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


class PceExpansionOrderCollocRatioLeastSquares(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocRatioLeastSquares"""

    least_squares: Union[
        ExpansionOrderCollocRatioLeastSquaresSvd,
        ExpansionOrderCollocRatioLeastSquaresEqCon,
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


class PceExpansionOrderCollocRatioOMP(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocRatioOMP"""

    orthogonal_matching_pursuit: ExpansionOrderCollocRatioOMPConfig = DakotaField(
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


class ExpansionOrderCollocRatioBPDN(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioBPDN"""

    basis_pursuit_denoising: ExpansionOrderCollocRatioBPDNConfig = DakotaField(
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


class ExpansionOrderCollocRatioLars(DakotaBaseModel):
    """Generated model for ExpansionOrderCollocRatioLars"""

    least_angle_regression: ExpansionOrderCollocRatioLarsConfig = DakotaField(
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


class PceExpansionOrderCollocRatioLasso(DakotaBaseModel):
    """Generated model for PceExpansionOrderCollocRatioLasso"""

    least_absolute_shrinkage: ExpansionOrderCollocRatioLassoConfig = DakotaField(
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


class PceOrthogLeastInterpImportBuildPointsFileCustomAnnotated(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpImportBuildPointsFileCustomAnnotated"""

    custom_annotated: PceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig = DakotaField(
        default_factory=PceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig,
        description="Selects custom-annotated tabular file format",
        dakota={
            "model_default": "PceOrthogLeastInterpImportBuildPointsFileCustomAnnotatedConfig",
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


class PceQuadratureOrder(DakotaBaseModel):
    """Generated model for PceQuadratureOrder"""

    quadrature_order: PceQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class PceSGLevel(DakotaBaseModel):
    """Generated model for PceSGLevel"""

    sparse_grid_level: PceSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
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
            ExpansionOrderCollocRatioBP,
            ExpansionOrderCollocRatioBPDN,
            ExpansionOrderCollocRatioLars,
            PceExpansionOrderCollocRatioLasso,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Regression Algorithm",
        dakota={"anchor": True, "union_pattern": 2},
    )
    cross_validation: ExpansionOrderCollocRatioCV | None = DakotaField(
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


class PceOrthogLeastInterpImportBuildPointsFile(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpImportBuildPointsFile"""

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
        PceOrthogLeastInterpImportBuildPointsFileCustomAnnotated,
        PceOrthogLeastInterpImportBuildPointsFileAnnotated,
        PceOrthogLeastInterpImportBuildPointsFileFreeform,
    ] = DakotaField(
        default_factory=PceOrthogLeastInterpImportBuildPointsFileAnnotated,
        description="Tabular Format",
        dakota={
            "anchor": True,
            "union_pattern": 1,
            "model_default": "PceOrthogLeastInterpImportBuildPointsFileAnnotated",
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


class PceOrthogLeastInterpConfig(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterpConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="pceorthogleastinterpconfig", list_field="tensor_grid"
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
    import_build_points_file: PceOrthogLeastInterpImportBuildPointsFile | None = (
        DakotaField(
            default=None,
            description="File containing points you wish to use to build a surrogate",
            dakota={"argument": "filename", "aliases": ["import_points_file"]},
        )
    )


class PceExpansionOrderConfig(DakotaBaseModel):
    """Generated model for PceExpansionOrderConfig"""

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


class PceOrthogLeastInterp(DakotaBaseModel):
    """Generated model for PceOrthogLeastInterp"""

    orthogonal_least_interpolation: PceOrthogLeastInterpConfig = DakotaField(
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


class PceExpansionOrder(DakotaBaseModel):
    """Generated model for PceExpansionOrder"""

    expansion_order: PceExpansionOrderConfig = DakotaField(
        default=...,
        description="The (initial) order of a polynomial expansion",
        dakota={"argument": "order"},
    )


class PceConfig(
    MethodThreeOptionalKeywordsMixin,
    PceRefinementMixin,
    MethodExpConvergenceToleranceMixin,
    PceOptionsMixin,
    ExpansionOptionsMixin,
    MethodSeedMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for PceConfig"""

    coefficient_approach: Union[
        PceQuadratureOrder,
        PceSGLevel,
        PceCubatureIntegrand,
        PceExpansionOrder,
        PceOrthogLeastInterp,
        PceImportExpansionFile,
    ] = DakotaField(
        description="Chaos coefficient estimation approach",
        dakota={"anchor": True, "union_pattern": 4},
    )


class PceSelection(MethodSelection):
    """Generated model for PceSelection"""

    polynomial_chaos: PceConfig = DakotaField(
        dakota={
            "aliases": ["nond_polynomial_chaos"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "POLYNOMIAL_CHAOS",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
