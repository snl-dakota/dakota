"""Generated Pydantic models for method.global_evidence"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    LevelMappingsNoRelContext1Mixin,
    MethodGlobalSubProblemSolverContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
    RngOptionsContext2Mixin,
)


class GlobalEvidenceConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodGlobalSubProblemSolverContext1Mixin,
    LevelMappingsNoRelContext1Mixin,
    RngOptionsContext2Mixin,
    MethodOptionalModelPointerMixin,
):
    "Evidence theory with evidence measures computed with global optimization methods"

    samples: int = DakotaField(
        default=0,
        description="Number of samples for sampling-based methods",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.samples",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
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


class GlobalEvidenceSelection(MethodSelection):
    "Generated model for GlobalEvidenceSelection"

    global_evidence: GlobalEvidenceConfig = DakotaField(
        dakota={
            "aliases": ["nond_global_evidence"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "GLOBAL_EVIDENCE",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
