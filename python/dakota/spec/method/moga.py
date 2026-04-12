"""Generated Pydantic models for method.moga"""

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


class LayerRank(DakotaBaseModel):
    """Generated model for LayerRank"""

    layer_rank: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "stored_value": "layer_rank",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DominationCount(DakotaBaseModel):
    """Generated model for DominationCount"""

    domination_count: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_type",
                    "stored_value": "domination_count",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MogaReplacementTypeElitist(DakotaBaseModel):
    """Generated model for MogaReplacementTypeElitist"""

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


class MogaReplacementTypeRouletteWheel(DakotaBaseModel):
    """Generated model for MogaReplacementTypeRouletteWheel"""

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


class MogaReplacementTypeUniqueRouletteWheel(DakotaBaseModel):
    """Generated model for MogaReplacementTypeUniqueRouletteWheel"""

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


class BelowLimitConfig(DakotaBaseModel):
    """Generated model for BelowLimitConfig"""

    threshhold: DakotaFloat = DakotaField(
        gt=0,
        description="Limit number of designs dominating those kept",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.replacement_type",
                    "stored_value": "below_limit",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.jega.fitness_limit",
                    "secondary_ir_value_type": "Real",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    shrinkage_fraction: DakotaFloat = DakotaField(
        default=0.9,
        ge=0,
        le=1,
        description="Decrease the population size by a percentage",
        dakota={
            "aliases": ["shrinkage_percentage"],
            "materialization": [
                {
                    "ir_key": "method.jega.shrinkage_percentage",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ],
        },
    )


class Radial(DakotaBaseModel):
    """Generated model for Radial"""

    radial: list[DakotaFloat] = DakotaField(
        description="Set niching distance to percentage of non-dominated range",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.niching_type",
                    "stored_value": "radial",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.jega.niche_vector",
                    "secondary_ir_value_type": "RealVector",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class NichingTypeDistance(DakotaBaseModel):
    """Generated model for NichingTypeDistance"""

    distance: list[DakotaFloat] = DakotaField(
        description="Enforce minimum Euclidean distance between designs",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.niching_type",
                    "stored_value": "distance",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.jega.niche_vector",
                    "secondary_ir_value_type": "RealVector",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class MaxDesignsConfig(DakotaBaseModel):
    """Generated model for MaxDesignsConfig"""

    min_distances: list[DakotaFloat] = DakotaField(
        description="Limit number of solutions to remain in the population",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.niching_type",
                    "stored_value": "max_designs",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.jega.niche_vector",
                    "secondary_ir_value_type": "RealVector",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    num_designs: int = DakotaField(
        default=100,
        ge=2,
        description="Limit the number of solutions",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.num_designs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "size_t",
                }
            ]
        },
    )


class ConvergenceType(DakotaBaseModel):
    """Generated model for ConvergenceType"""

    metric_tracker: Literal[True] = DakotaField(
        description="Track changes in the non-dominated frontier",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.convergence_type",
                    "stored_value": "metric_tracker",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )
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


class PostprocessorType(DakotaBaseModel):
    """Generated model for PostprocessorType"""

    orthogonal_distance: list[DakotaFloat] = DakotaField(
        description="Get subset of Pareto front based on distance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.jega.postprocessor_type",
                    "stored_value": "distance_postprocessor",
                    "storage_type": "TYPE_DATA_COMBINED",
                    "secondary_ir_key": "method.jega.distance_vector",
                    "secondary_ir_value_type": "RealVector",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BelowLimit(DakotaBaseModel):
    """Generated model for BelowLimit"""

    below_limit: BelowLimitConfig = DakotaField(
        default=...,
        description="Limit number of designs dominating those kept",
        dakota={"argument": "threshhold"},
    )


class MaxDesigns(DakotaBaseModel):
    """Generated model for MaxDesigns"""

    max_designs: MaxDesignsConfig = DakotaField(
        default=...,
        description="Limit number of solutions to remain in the population",
        dakota={"argument": "min_distances"},
    )


class MogaConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodJegaCommonOptsMixin,
):
    """Generated model for MogaConfig"""

    fitness_type: Union[LayerRank, DominationCount] | None = DakotaField(
        default=None,
        description="Select the fitness type for JEGA methods",
        dakota={"union_pattern": 2},
    )
    replacement_type: (
        Union[
            MogaReplacementTypeElitist,
            MogaReplacementTypeRouletteWheel,
            MogaReplacementTypeUniqueRouletteWheel,
            BelowLimit,
        ]
        | None
    ) = DakotaField(
        default=None,
        description="Select a replacement type for JEGA methods",
        dakota={"union_pattern": 2},
    )
    niching_type: Union[Radial, NichingTypeDistance, MaxDesigns] | None = DakotaField(
        default=None,
        description="Specify the type of niching pressure",
        dakota={"union_pattern": 2},
    )
    convergence_type: ConvergenceType | None = DakotaField(
        default=None, description="Select the convergence type for JEGA methods"
    )
    postprocessor_type: PostprocessorType | None = DakotaField(
        default=None, description="Post process the final solution from ``moga``"
    )


class MogaSelection(MethodSelection):
    """Generated model for MogaSelection"""

    moga: MogaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "MOGA",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
