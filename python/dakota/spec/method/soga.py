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
    "Select the fitness type for JEGA methods"

    merit_function: Literal[True] = DakotaField(
        description="Balance goals of reducing objective function and satisfying constraints",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "merit_function",
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
    "Use the best designs to form a new population"

    elitist: Literal[True] = DakotaField(
        default=True,
        description="Use the best designs to form a new population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "elitist",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class FavorFeasible(DakotaBaseModel):
    "Prioritize feasible designs"

    favor_feasible: Literal[True] = DakotaField(
        default=True,
        description="Prioritize feasible designs",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "favor_feasible",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SogaReplacementTypeRouletteWheel(DakotaBaseModel):
    "Replace population"

    roulette_wheel: Literal[True] = DakotaField(
        default=True,
        description="Replace population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "roulette_wheel",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class SogaReplacementTypeUniqueRouletteWheel(DakotaBaseModel):
    "Replace population"

    unique_roulette_wheel: Literal[True] = DakotaField(
        default=True,
        description="Replace population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "unique_roulette_wheel",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BestFitnessTrackerConfig(DakotaBaseModel):
    "Tracks the best fitness of the population"

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
    "Tracks the average fitness of the population"

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
    "Tracks the best fitness of the population"

    best_fitness_tracker: BestFitnessTrackerConfig = DakotaField(
        description="Tracks the best fitness of the population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.convergence_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "best_fitness_tracker",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AverageFitnessTracker(DakotaBaseModel):
    "Tracks the average fitness of the population"

    average_fitness_tracker: AverageFitnessTrackerConfig = DakotaField(
        description="Tracks the average fitness of the population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.convergence_type",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "average_fitness_tracker",
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
    "Single-objective Genetic Algorithm (a.k.a Evolutionary Algorithm)"

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
    "Generated model for SogaSelection"

    soga: SogaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SOGA",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
