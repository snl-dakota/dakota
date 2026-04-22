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


class StochCollocQuadratureOrderNonNested(DakotaBaseModel):
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


class StochCollocSGLevelNodal(DakotaBaseModel):
    "Employ a nodal sparse grid construction in stochastic collocation"

    nodal: Literal[True] = DakotaField(
        default=True,
        description="Employ a nodal sparse grid construction in stochastic collocation",
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


class StochCollocSGLevelHierarchical(DakotaBaseModel):
    "Employ a hierarchical sparse grid construction"

    hierarchical: Literal[True] = DakotaField(
        default=True,
        description="Employ a hierarchical sparse grid construction",
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


class StochCollocSGLevelRestricted(DakotaBaseModel):
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


class StochCollocSGLevelUnrestricted(DakotaBaseModel):
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


class StochCollocSGLevelNested(DakotaBaseModel):
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


class StochCollocSGLevelNonNested(DakotaBaseModel):
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


class StochCollocQuadratureOrderConfig(DakotaBaseModel):
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
        Union[StochCollocQuadratureOrderNested, StochCollocQuadratureOrderNonNested]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class StochCollocSGLevelConfig(DakotaBaseModel):
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
    "Order for tensor-products of Gaussian quadrature rules"

    quadrature_order: StochCollocQuadratureOrderConfig = DakotaField(
        default=...,
        description="Order for tensor-products of Gaussian quadrature rules",
        dakota={"argument": "order"},
    )


class StochCollocSGLevel(DakotaBaseModel):
    "Level to use in sparse grid integration or interpolation"

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
    "Uncertainty quantification with stochastic collocation"

    grid_type: Union[StochCollocQuadratureOrder, StochCollocSGLevel] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )


class StochCollocSelection(MethodSelection):
    "Generated model for StochCollocSelection"

    stoch_collocation: StochCollocConfig = DakotaField(
        dakota={
            "aliases": ["nond_stoch_collocation"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "STOCH_COLLOCATION",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
