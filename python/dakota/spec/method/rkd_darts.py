"""Generated Pydantic models for method.rkd_darts"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    LevelMappingsNoRelContext2Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext1Mixin,
)


class RkdDartsLipschitzLocalEst(DakotaBaseModel):
    """Generated model for RkdDartsLipschitzLocalEst"""

    local_est: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.lipschitz",
                    "stored_value": "local",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RkdDartsLipschitzGlobalEst(DakotaBaseModel):
    """Generated model for RkdDartsLipschitzGlobalEst"""

    global_est: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.lipschitz",
                    "stored_value": "global",
                    "storage_type": "PRESENCE_LITERAL",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class RkdDartsConfig(
    MethodThreeOptionalKeywordsMixin,
    LevelMappingsNoRelContext2Mixin,
    RngOptionsContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for RkdDartsConfig"""

    build_samples: int = DakotaField(
        description="Number of initial model evaluations used in build phase",
        dakota={
            "aliases": ["samples"],
            "materialization": [
                {
                    "ir_key": "method.build_samples",
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
    lipschitz: Union[RkdDartsLipschitzLocalEst, RkdDartsLipschitzGlobalEst] = (
        DakotaField(
            default_factory=RkdDartsLipschitzLocalEst,
            description="Undocumented: Recursive k-d (RKD) Darts is an experimental capability.",
            dakota={"union_pattern": 1, "model_default": "RkdDartsLipschitzLocalEst"},
        )
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


class RkdDartsSelection(MethodSelection):
    """Generated model for RkdDartsSelection"""

    rkd_darts: RkdDartsConfig = DakotaField(
        dakota={
            "aliases": ["nond_rkd_darts"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "RKD_DARTS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
