"""Generated Pydantic models for method.importance_sampling"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    LevelMappingsNoRelContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)


class ImportanceSamplingImportance(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    importance: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "IS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportanceSamplingAdaptImport(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    adapt_import: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "AIS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportanceSamplingMmAdaptImport(DakotaBaseModel):
    "Importance sampling option for probability refinement"

    mm_adapt_import: Literal[True] = DakotaField(
        default=True,
        description="Importance sampling option for probability refinement",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MMAIS",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportanceSamplingConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodMaxIterationsContext1Mixin,
    MethodConvergenceTolMixin,
    LevelMappingsNoRelContext1Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "Importance sampling"

    samples: int = DakotaField(
        default=0,
        description="Number of samples for sampling-based methods",
        dakota={
            "aliases": ["initial_samples"],
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
    approach: Union[
        ImportanceSamplingImportance,
        ImportanceSamplingAdaptImport,
        ImportanceSamplingMmAdaptImport,
    ] = DakotaField(
        description="Importance Sampling Approach",
        dakota={"anchor": True, "union_pattern": 4},
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


class ImportanceSamplingSelection(MethodSelection):
    "Generated model for ImportanceSamplingSelection"

    importance_sampling: ImportanceSamplingConfig = DakotaField(
        dakota={
            "aliases": ["nond_importance_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "IMPORTANCE_SAMPLING",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
