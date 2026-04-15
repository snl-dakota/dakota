"""Generated Pydantic models for method.multifidelity_stoch_collocation"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from typing import ClassVar, List
from ..validation import ValidationRule
from ..validation.rules import CheckNonnegativeList

# Cross-module model imports
from dakota.spec.shared.core import MethodMfExpConvergenceToleranceMixin
from dakota.spec.shared.expansion.options import ExpansionOptionsMixin
from dakota.spec.shared.misc import (
    DiscrepEmulationMixin,
    MethodOptionalModelPointerMixin,
    MethodSeedSequenceMixin,
    MethodThreeOptionalKeywordsMixin,
    MfAllocControlContext1Mixin,
    ScOptionsMixin,
    ScRefinementMixin,
)


class MultifidelityStochCollocQuadratureOrderSequenceNested(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocQuadratureOrderSequenceNested"""

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


class MultifidelityStochCollocQuadratureOrderSequenceNonNested(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocQuadratureOrderSequenceNonNested"""

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


class MultifidelityStochCollocSGLevelSequenceNodal(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceNodal"""

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


class MultifidelityStochCollocSGLevelSequenceHierarchical(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceHierarchical"""

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


class MultifidelityStochCollocSGLevelSequenceRestricted(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceRestricted"""

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


class MultifidelityStochCollocSGLevelSequenceUnrestricted(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceUnrestricted"""

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


class MultifidelityStochCollocSGLevelSequenceNested(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceNested"""

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


class MultifidelityStochCollocSGLevelSequenceNonNested(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceNonNested"""

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


class MultifidelityStochCollocQuadratureOrderSequenceConfig(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocQuadratureOrderSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="multifidelitystochcollocquadratureordersequenceconfig",
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
        Union[
            MultifidelityStochCollocQuadratureOrderSequenceNested,
            MultifidelityStochCollocQuadratureOrderSequenceNonNested,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MultifidelityStochCollocSGLevelSequenceConfig(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequenceConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="multifidelitystochcollocsglevelsequenceconfig",
            list_field="sequence",
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
        Union[
            MultifidelityStochCollocSGLevelSequenceNodal,
            MultifidelityStochCollocSGLevelSequenceHierarchical,
        ]
        | None
    ) = DakotaField(default=None, dakota={"anchor": True, "union_pattern": 2})
    growth_rule: (
        Union[
            MultifidelityStochCollocSGLevelSequenceRestricted,
            MultifidelityStochCollocSGLevelSequenceUnrestricted,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Growth",
        dakota={"anchor": True, "union_pattern": 2},
    )
    nesting_rule: (
        Union[
            MultifidelityStochCollocSGLevelSequenceNested,
            MultifidelityStochCollocSGLevelSequenceNonNested,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Quadrature Rule Nesting",
        dakota={"anchor": True, "union_pattern": 2},
    )


class MultifidelityStochCollocQuadratureOrderSequence(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocQuadratureOrderSequence"""

    quadrature_order_sequence: MultifidelityStochCollocQuadratureOrderSequenceConfig = (
        DakotaField(
            default=...,
            description="Sequence of quadrature orders used in a multi-stage expansion",
            dakota={"argument": "sequence"},
        )
    )


class MultifidelityStochCollocSGLevelSequence(DakotaBaseModel):
    """Generated model for MultifidelityStochCollocSGLevelSequence"""

    sparse_grid_level_sequence: MultifidelityStochCollocSGLevelSequenceConfig = DakotaField(
        default=...,
        description="Sequence of sparse grid levels used in a multi-stage expansion",
        dakota={"argument": "sequence"},
    )


class MultifidelityStochCollocConfig(
    MethodThreeOptionalKeywordsMixin,
    ScRefinementMixin,
    MethodMfExpConvergenceToleranceMixin,
    MfAllocControlContext1Mixin,
    DiscrepEmulationMixin,
    ScOptionsMixin,
    ExpansionOptionsMixin,
    MethodSeedSequenceMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for MultifidelityStochCollocConfig"""

    grid_type: Union[
        MultifidelityStochCollocQuadratureOrderSequence,
        MultifidelityStochCollocSGLevelSequence,
    ] = DakotaField(
        description="Interpolation Grid Type",
        dakota={"anchor": True, "union_pattern": 4},
    )


class MultifidelityStochCollocSelection(MethodSelection):
    """Generated model for MultifidelityStochCollocSelection"""

    multifidelity_stoch_collocation: MultifidelityStochCollocConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTIFIDELITY_STOCH_COLLOCATION",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
