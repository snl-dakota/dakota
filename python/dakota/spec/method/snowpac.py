"""Generated Pydantic models for method.snowpac"""

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


class SnowpacConfig(
    MethodThreeOptionalKeywordsMixin,
    DefaultTrustRegionContext2Mixin,
    MethodMaxIterationsContext1Mixin,
    MethodMaxFunctionEvaluationsContext1Mixin,
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
):
    "Stochastic version of NOWPAC that incorporates error estimates and noise mitigation."

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


class SnowpacSelection(MethodSelection):
    "Generated model for SnowpacSelection"

    snowpac: SnowpacConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MIT_SNOWPAC",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
