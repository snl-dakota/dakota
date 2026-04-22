"""Generated Pydantic models for method.pof_darts"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    LevelMappingsNoRelContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)


class PofDartsLipschitzLocalEst(DakotaBaseModel):
    "Specify local estimation of the Lipschitz constant"

    local_est: Literal[True] = DakotaField(
        default=True,
        description="Specify local estimation of the Lipschitz constant",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.lipschitz",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "local",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class PofDartsLipschitzGlobalEst(DakotaBaseModel):
    "Specify global estimation of the Lipschitz estimate"

    global_est: Literal[True] = DakotaField(
        default=True,
        description="Specify global estimation of the Lipschitz estimate",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.lipschitz",
                    "storage_type": "PRESENCE_LITERAL",
                    "stored_value": "global",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class PofDartsConfig(
    MethodThreeOptionalKeywordsMixin,
    LevelMappingsNoRelContext1Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "Probability-of-Failure (POF) darts is a novel method for estimating the probability of failure based on random sphere-packing."

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
    lipschitz: Union[PofDartsLipschitzLocalEst, PofDartsLipschitzGlobalEst] = (
        DakotaField(
            default_factory=PofDartsLipschitzLocalEst,
            description="Select the type of Lipschitz estimation (global or local)",
            dakota={"union_pattern": 1, "model_default": "PofDartsLipschitzLocalEst"},
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


class PofDartsSelection(MethodSelection):
    "Generated model for PofDartsSelection"

    pof_darts: PofDartsConfig = DakotaField(
        dakota={
            "aliases": ["nond_pof_darts"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "POF_DARTS",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
