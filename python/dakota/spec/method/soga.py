"""Generated Pydantic models for method.soga"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union
from ..base import DBL_MAX

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodJegaCommonOptsMixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodThreeOptionalKeywordsMixin,
)


class FitnessType(DakotaBaseModel):
    """Generated model for FitnessType"""

    merit_function: Literal[True] = DakotaField(
        description="Balance goals of reducing objective function and satisfying constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "stored_value": "merit_function",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    constraint_penalty: DakotaFloat = DakotaField(
        default=0.0,
        gt=0,
        description="Multiplier for the penalty function",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.constraint_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class SogaReplacementTypeElitist(DakotaBaseModel):
    """Generated model for SogaReplacementTypeElitist"""

    elitist: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "elitist",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class FavorFeasible(DakotaBaseModel):
    """Generated model for FavorFeasible"""

    favor_feasible: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "favor_feasible",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SogaReplacementTypeRouletteWheel(DakotaBaseModel):
    """Generated model for SogaReplacementTypeRouletteWheel"""

    roulette_wheel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "roulette_wheel",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SogaReplacementTypeUniqueRouletteWheel(DakotaBaseModel):
    """Generated model for SogaReplacementTypeUniqueRouletteWheel"""

    unique_roulette_wheel: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "unique_roulette_wheel",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BestFitnessTrackerConfig(DakotaBaseModel):
    """Generated model for BestFitnessTrackerConfig"""

    percent_change: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        ge=0,
        description="Define the convergence criterion for JEGA methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    num_generations: int = DakotaField(
        default=15,
        ge=0,
        description="Define the convergence criterion for JEGA methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.num_generations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class AverageFitnessTrackerConfig(DakotaBaseModel):
    """Generated model for AverageFitnessTrackerConfig"""

    percent_change: DakotaFloat = DakotaField(
        default=-DBL_MAX,
        ge=0,
        description="Define the convergence criterion for JEGA methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.convergence_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
                {
                    "ir_key": "method.jega.percent_change",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                },
            ]
        },
    )
    num_generations: int = DakotaField(
        default=15,
        ge=0,
        description="Define the convergence criterion for JEGA methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.num_generations",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class BestFitnessTracker(DakotaBaseModel):
    """Generated model for BestFitnessTracker"""

    best_fitness_tracker: BestFitnessTrackerConfig = DakotaField(
        description="Tracks the best fitness of the population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.convergence_type",
                    "stored_value": "best_fitness_tracker",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AverageFitnessTracker(DakotaBaseModel):
    """Generated model for AverageFitnessTracker"""

    average_fitness_tracker: AverageFitnessTrackerConfig = DakotaField(
        description="Tracks the average fitness of the population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.convergence_type",
                    "stored_value": "average_fitness_tracker",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SogaConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodJegaCommonOptsMixin,
):
    """Generated model for SogaConfig"""

    fitness_type: FitnessType | None = DakotaField(
        default=None, description="Select the fitness type for JEGA methods"
    )
    replacement_type: (
        Union[
            SogaReplacementTypeElitist,
            FavorFeasible,
            SogaReplacementTypeRouletteWheel,
            SogaReplacementTypeUniqueRouletteWheel,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Select a replacement type for JEGA methods",
        dakota={"union_pattern": 2},
    )
    convergence_type: Union[BestFitnessTracker, AverageFitnessTracker] | None = (
        DakotaField(
            default=None,
            description="Select the convergence type for JEGA methods",
            dakota={"union_pattern": 2},
        )
    )


class SogaSelection(MethodSelection):
    """Generated model for SogaSelection"""

    soga: SogaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "SOGA",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
