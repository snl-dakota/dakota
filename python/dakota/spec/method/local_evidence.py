"""Generated Pydantic models for method.local_evidence"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    LevelMappingsNoRelContext1Mixin,
    MethodGradientSubProblemSolverMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class LocalEvidenceConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodGradientSubProblemSolverMixin,
    LevelMappingsNoRelContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    "Evidence theory with evidence measures computed with local optimization methods"

    pass


class LocalEvidenceSelection(MethodSelection):
    "Generated model for LocalEvidenceSelection"

    local_evidence: LocalEvidenceConfig = DakotaField(
        dakota={
            "aliases": ["nond_local_evidence"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "LOCAL_EVIDENCE",
                    "ir_value_type": "unsigned short",
                }
            ],
        }
    )
