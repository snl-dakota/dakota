"""Generated Pydantic models for method.nowpac"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    DefaultTrustRegionContext2Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class NowpacConfig(
    MethodThreeOptionalKeywordsMixin,
    DefaultTrustRegionContext2Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for NowpacConfig"""

    pass


class NowpacSelection(MethodSelection):
    """Generated model for NowpacSelection"""

    nowpac: NowpacConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MIT_NOWPAC",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
