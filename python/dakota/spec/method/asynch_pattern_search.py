"""Generated Pydantic models for method.asynch_pattern_search"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from ..base import DBL_MAX

# Cross-module model imports
from dakota.spec.shared.core import DefaultConstraintTolMixin
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodSynchronizationMixin,
    MethodThreeOptionalKeywordsMixin,
)


class MeritMax(DakotaBaseModel):
    "Nonsmoothed merit function"

    merit_max: Literal[True] = DakotaField(
        default=True,
        description="Nonsmoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit_max",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MeritMaxSmooth(DakotaBaseModel):
    "Smoothed merit function"

    merit_max_smooth: Literal[True] = DakotaField(
        default=True,
        description="Smoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit_max_smooth",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Merit1(DakotaBaseModel):
    "Nonsmoothed merit function"

    merit1: Literal[True] = DakotaField(
        default=True,
        description="Nonsmoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit1",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Merit1Smooth(DakotaBaseModel):
    "Smoothed merit function"

    merit1_smooth: Literal[True] = DakotaField(
        default=True,
        description="Smoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit1_smooth",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Merit2(DakotaBaseModel):
    "Nonsmoothed merit function"

    merit2: Literal[True] = DakotaField(
        default=True,
        description="Nonsmoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit2",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Merit2Smooth(DakotaBaseModel):
    "Smoothed merit function"

    merit2_smooth: Literal[True] = DakotaField(
        default=True,
        description="Smoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit2_smooth",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Merit2Squared(DakotaBaseModel):
    "Nonsmoothed merit function"

    merit2_squared: Literal[True] = DakotaField(
        default=True,
        description="Nonsmoothed merit function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit2_squared",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AsynchPatternSearchConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodSynchronizationMixin,
    DefaultConstraintTolMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    "Pattern search, derivative free optimization method"

    initial_delta: DakotaFloat = DakotaField(
        default=1.0,
        description="Initial step size for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.initial_delta",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    contraction_factor: DakotaFloat = DakotaField(
        default=0.5,
        description="Amount by which step length is rescaled",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.contraction_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    variable_tolerance: DakotaFloat = DakotaField(
        default=0.01,
        description="Step length-based stopping criteria for derivative-free optimizers",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    solution_target: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        description="Stopping criteria based on objective function value",
        dakota={
            "aliases": ["solution_accuracy"],
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.solution_target",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.solution_target",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ],
        },
    )
    merit_function: Union[
        MeritMax,
        MeritMaxSmooth,
        Merit1,
        Merit1Smooth,
        Merit2,
        Merit2Smooth,
        Merit2Squared,
    ] = DakotaField(
        default_factory=Merit2Squared,
        description="Balance goals of reducing objective function and satisfying constraints",
        dakota={"union_pattern": 1, "model_default": "Merit2Squared"},
    )
    constraint_penalty: DakotaFloat = DakotaField(
        default=1.0,
        description="Multiplier for the penalty function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.constraint_penalty",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    smoothing_factor: DakotaFloat = DakotaField(
        default=0.0,
        description="Smoothing value for smoothed penalty functions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.asynch_pattern_search.smoothing_factor",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class AsynchPatternSearchSelection(MethodSelection):
    "Generated model for AsynchPatternSearchSelection"

    asynch_pattern_search: AsynchPatternSearchConfig = DakotaField(
        dakota={
            "aliases": ["coliny_apps"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ASYNCH_PATTERN_SEARCH",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
