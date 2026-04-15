"""Generated Pydantic models for shared.interface"""

from __future__ import annotations

from ...base import DakotaBaseModel, DakotaField
from typing import Literal, Union


class MethodSystemReliabilitySeries(DakotaBaseModel):
    """Generated model for MethodSystemReliabilitySeries"""

    series: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MethodSystemReliabilityParallel"""

    parallel: Literal[True] = DakotaField(
        default=True,
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
    """Generated model for MethodSystemReliabilityMixin"""

    system: (
        Union[MethodSystemReliabilitySeries, MethodSystemReliabilityParallel] | None
    ) = DakotaField(
        default=None,
        description="Compute system reliability (series or parallel)",
        dakota={"union_pattern": 2},
    )
