"""Generated Pydantic models for shared.interface"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField
from typing import Literal, Union


class MethodSystemReliabilitySeries(DakotaBaseModel):
    "Aggregate response statistics assuming a series system"

    series: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a series system",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_SERIES",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodSystemReliabilityParallel(DakotaBaseModel):
    "Aggregate response statistics assuming a parallel system"

    parallel: Literal[True] = DakotaField(
        default=True,
        description="Aggregate response statistics assuming a parallel system",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.nond.response_level_target_reduce",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SYSTEM_PARALLEL",
                    "ir_value_type": "short",
                }
            ]
        },
    )


class MethodSystemReliabilityMixin(DakotaBaseModel):
    "Generated model for MethodSystemReliabilityMixin"

    system: (
        Union[MethodSystemReliabilitySeries, MethodSystemReliabilityParallel] | None
    ) = DakotaField(
        default=None,
        description="Compute system reliability (series or parallel)",
        dakota={"union_pattern": 2},
    )
