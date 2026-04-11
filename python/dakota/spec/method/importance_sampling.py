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
    """Generated model for ImportanceSamplingImportance"""

    importance: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "stored_value": "IS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportanceSamplingAdaptImport(DakotaBaseModel):
    """Generated model for ImportanceSamplingAdaptImport"""

    adapt_import: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "stored_value": "AIS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ImportanceSamplingMmAdaptImport(DakotaBaseModel):
    """Generated model for ImportanceSamplingMmAdaptImport"""

    mm_adapt_import: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.integration_refinement",
                    "stored_value": "MMAIS",
                    "storage_type": "PRESENCE_ENUM",
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
    """Generated model for ImportanceSamplingConfig"""

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
    """Generated model for ImportanceSamplingSelection"""

    importance_sampling: ImportanceSamplingConfig = DakotaField(
        dakota={
            "aliases": ["nond_importance_sampling"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "IMPORTANCE_SAMPLING",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
