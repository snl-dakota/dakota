"""Generated Pydantic models for method.coliny_pattern_search"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
    MethodSynchronizationMixin,
    MethodThreeOptionalKeywordsMixin,
)


class Coordinate(DakotaBaseModel):
    "Use coordinate directions as search pattern"

    coordinate: Literal[True] = DakotaField(
        default=True,
        description="Use coordinate directions as search pattern",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.pattern_basis",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "coordinate",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Simplex(DakotaBaseModel):
    "Use a minimal simplex for the search pattern"

    simplex: Literal[True] = DakotaField(
        default=True,
        description="Use a minimal simplex for the search pattern",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.pattern_basis",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "simplex",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MultiStep(DakotaBaseModel):
    "Examine trial step around successful new point"

    multi_step: Literal[True] = DakotaField(
        default=True,
        description="Examine trial step around successful new point",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.exploratory_moves",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "multi_step",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AdaptivePattern(DakotaBaseModel):
    "Adaptively rescale search directions"

    adaptive_pattern: Literal[True] = DakotaField(
        default=True,
        description="Adaptively rescale search directions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.exploratory_moves",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "adaptive",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BasicPattern(DakotaBaseModel):
    "Use the same search pattern every iteration"

    basic_pattern: Literal[True] = DakotaField(
        default=True,
        description="Use the same search pattern every iteration",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.exploratory_moves",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "simple",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ColinyPatternSearchConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSynchronizationMixin,
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
):
    "Pattern search, derivative free optimization method"

    constant_penalty: Literal[True] | None = DakotaField(
        default=None,
        description="Use a simple weighted penalty to manage feasibility",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.constant_penalty",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    no_expansion: Literal[True] | None = DakotaField(
        default=None,
        description="Don't allow expansion of the search pattern",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.expansion",
                    "storage_type": "PRESENCE_FALSE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    expand_after_success: int = DakotaField(
        default=0,
        description="Set the factor by which a search pattern can be expanded",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.expand_after_success",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    pattern_basis: Union[Coordinate, Simplex] | None = DakotaField(
        default=None, description="Pattern basis selection", dakota={"union_pattern": 2}
    )
    stochastic: Literal[True] | None = DakotaField(
        default=None,
        description="Generate trial points in random order",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.randomize",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )
    total_pattern_size: int = DakotaField(
        default=0,
        description="Total number of points in search pattern",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.total_pattern_size",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    exploratory_moves: Union[MultiStep, AdaptivePattern, BasicPattern] | None = (
        DakotaField(
            default=None,
            description="Exploratory moves selection",
            dakota={"union_pattern": 2},
        )
    )
    contraction_factor: DakotaFloat = DakotaField(
        default=0.5,
        description="Amount by which step length is rescaled",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.contraction_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    constraint_penalty: DakotaFloat = DakotaField(
        default=-1.0,
        description="Multiplier for the penalty function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.constraint_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    initial_delta: DakotaFloat = DakotaField(
        default=-1.0,
        description="Initial step size for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.initial_delta",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    variable_tolerance: DakotaFloat = DakotaField(
        default=-1.0,
        description="Step length-based stopping criteria for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class ColinyPatternSearchSelection(MethodSelection):
    "Generated model for ColinyPatternSearchSelection"

    coliny_pattern_search: ColinyPatternSearchConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COLINY_PATTERN_SEARCH",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
