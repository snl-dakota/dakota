"""Generated Pydantic models for method.stoch_collocation"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from ..base import USHRT_MAX

# Cross-module model imports
from dakota.spec.shared.core import MethodExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodSeedMixin,
    MethodThreeOptionalKeywordsMixin,
    ScOptionsMixin,
    ScRefinementMixin,
)


class StochCollocQuadratureOrderNested(DakotaBaseModel):
    """Generated model for StochCollocQuadratureOrderNested"""

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


class StochCollocQuadratureOrderNonNested(DakotaBaseModel):
    """Generated model for StochCollocQuadratureOrderNonNested"""

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


class StochCollocSGLevelNodal(DakotaBaseModel):
    """Generated model for StochCollocSGLevelNodal"""

    nodal: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "stored_value": "NODAL_INTERPOLANT",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class StochCollocSGLevelHierarchical(DakotaBaseModel):
    """Generated model for StochCollocSGLevelHierarchical"""

    hierarchical: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.expansion_basis_type",
                    "stored_value": "HIERARCHICAL_INTERPOLANT",
                    "storage_type": "PRESENCE_ENUM",
                    "enum_scope": "Pecos",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class StochCollocSGLevelRestricted(DakotaBaseModel):
    """Generated model for StochCollocSGLevelRestricted"""

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


class StochCollocSGLevelUnrestricted(DakotaBaseModel):
    """Generated model for StochCollocSGLevelUnrestricted"""

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


class StochCollocSGLevelNested(DakotaBaseModel):
    """Generated model for StochCollocSGLevelNested"""

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


class StochCollocSGLevelNonNested(DakotaBaseModel):
    """Generated model for StochCollocSGLevelNonNested"""

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


class StochCollocQuadratureOrderConfig(DakotaBaseModel):
    """Generated model for StochCollocQuadratureOrderConfig"""

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
        Union[StochCollocQuadratureOrderNested, StochCollocQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class StochCollocSGLevelConfig(DakotaBaseModel):
    """Generated model for StochCollocSGLevelConfig"""

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
    interpolant: (
        Union[StochCollocSGLevelNodal, StochCollocSGLevelHierarchical] | None
    ) = DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    growth_rule: (
        Union[StochCollocSGLevelRestricted, StochCollocSGLevelUnrestricted] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[StochCollocSGLevelNested, StochCollocSGLevelNonNested] | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class StochCollocQuadratureOrder(DakotaBaseModel):
    """Generated model for StochCollocQuadratureOrder"""

    quadrature_order: StochCollocQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class StochCollocSGLevel(DakotaBaseModel):
    """Generated model for StochCollocSGLevel"""

    sparse_grid_level: StochCollocSGLevelConfig = DakotaField(
        default=...,
        description="Level to use in sparse grid integration or interpolation",
        dakota={"argument": "level"},
    )


class StochCollocConfig(
    MethodThreeOptionalKeywordsMixin,
    ScRefinementMixin,
    MethodExpConvergenceToleranceMixin,
    ScOptionsMixin,
    ExpansionOptionsMixin,
    MethodSeedMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for StochCollocConfig"""

    grid_type: Union[StochCollocQuadratureOrder, StochCollocSGLevel] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )


class StochCollocSelection(MethodSelection):
    """Generated model for StochCollocSelection"""

    stoch_collocation: StochCollocConfig = DakotaField(
        dakota={
            "aliases": ["nond_stoch_collocation"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "STOCH_COLLOCATION",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
