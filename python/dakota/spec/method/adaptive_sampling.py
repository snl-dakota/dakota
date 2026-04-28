"""Generated Pydantic models for method.adaptive_sampling"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    ImportBuildMixin,
    LevelMappingsNoRelContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)
from dakota.spec.shared.surrogate import MethodExportApproxFormatMixin


class PredictedVariance(DakotaBaseModel):
    "Pick points with highest variance"

    predicted_variance: Literal[True] = DakotaField(
        default=True,
        description="Pick points with highest variance",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_metric",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "predicted_variance",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class FitnessMetricDistance(DakotaBaseModel):
    "Space filling metric"

    distance: Literal[True] = DakotaField(
        default=True,
        description="Space filling metric",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_metric",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "distance",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Gradient(DakotaBaseModel):
    "Fill the range space of the surrogate"

    gradient: Literal[True] = DakotaField(
        default=True,
        description="Fill the range space of the surrogate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.fitness_metric",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "gradient",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Naive(DakotaBaseModel):
    "Take the highest scoring candidates"

    naive: Literal[True] = DakotaField(
        default=True,
        description="Take the highest scoring candidates",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_selection",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "naive",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class DistancePenalty(DakotaBaseModel):
    "Add a penalty to spread out the points in the batch"

    distance_penalty: Literal[True] = DakotaField(
        default=True,
        description="Add a penalty to spread out the points in the batch",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_selection",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "distance",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class Topology(DakotaBaseModel):
    "In this selection strategy, we use information about the topology of the space from the Morse-Smale complex to identify next points to select."

    topology: Literal[True] = DakotaField(
        default=True,
        description="In this selection strategy, we use information about the topology of the space from the Morse-Smale complex to identify next points to select.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_selection",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "topology",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class ConstantLiar(DakotaBaseModel):
    "Use information from the existing surrogate model to predict what the surrogate upgrade will be with new points."

    constant_liar: Literal[True] = DakotaField(
        default=True,
        description="Use information from the existing surrogate model to predict what the surrogate upgrade will be with new points.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.batch_selection",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "cl",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AdaptiveSamplingExportApproxPointsFile(MethodExportApproxFormatMixin):
    "Output file for surrogate model value evaluations"

    filename: str = DakotaField(
        description="Output file for surrogate model value evaluations",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.export_approx_points_file",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class AdaptiveSamplingConfig(
    MethodThreeOptionalKeywordsMixin,
    ImportBuildMixin,
    MethodMaxIterationsContext1Mixin,
    LevelMappingsNoRelContext1Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "(Experimental) Adaptively refine a Gaussian process surrogate"

    initial_samples: int = DakotaField(
        default=0,
        description="Initial number of samples for sampling-based methods",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ],
        },
    )
    seed: int | None = DakotaField(
        default=None,
        gt=0,
        description="Seed of the random number generator",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.random_seed",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    samples_on_emulator: int = DakotaField(
        default=0,
        description="Number of samples at which to evaluate an emulator (surrogate)",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.samples_on_emulator",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    fitness_metric: Union[PredictedVariance, FitnessMetricDistance, Gradient] = (
        DakotaField(
            default_factory=PredictedVariance,
            description="(Experimental) Specify the ``fitness_metric`` used to select the next point",
            dakota={"union_pattern": 1, "model_default": "PredictedVariance"},
        )
    )
    batch_selection: Union[Naive, DistancePenalty, Topology, ConstantLiar] = (
        DakotaField(
            default_factory=Naive,
            description="(Experimental) How to select new points",
            dakota={"union_pattern": 1, "model_default": "Naive"},
        )
    )
    refinement_samples: list[int] | None = DakotaField(
        default=None,
        description="Number of samples used to refine a probability estimate or sampling design.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.refinement_samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ]
        },
    )
    export_approx_points_file: AdaptiveSamplingExportApproxPointsFile | None = (
        DakotaField(
            default=None,
            description="Output file for surrogate model value evaluations",
            dakota={"argument": "filename", "aliases": ["export_points_file"]},
        )
    )
    misc_options: list[str] | None = DakotaField(
        default=None,
        description="(Experimental) Hook for algorithm-specific adaptive sampling options",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.misc_options",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )


class AdaptiveSamplingSelection(MethodSelection):
    "Generated model for AdaptiveSamplingSelection"

    adaptive_sampling: AdaptiveSamplingConfig = DakotaField(
        dakota={
            "aliases": ["nond_adaptive_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "ADAPTIVE_SAMPLING",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
