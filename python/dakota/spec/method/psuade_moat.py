"""Generated Pydantic models for method.psuade_moat"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection
from typing import ClassVar, List
from ..validation import ValidationRule
from ..validation.rules import CheckNonnegativeList

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class PsuadeMoatConfig(
    MethodThreeOptionalKeywordsMixin, MethodOptionalModelPointerMixin
):
    """Generated model for PsuadeMoatConfig"""

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(context="psuademoatconfig", list_field="partitions"),
    ]

    partitions: list[int] | None = DakotaField(
        default=None,
        description="Number of partitions of each variable",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.partitions",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "UShortArray",
                }
            ]
        },
    )
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


class PsuadeMoatSelection(MethodSelection):
    """Generated model for PsuadeMoatSelection"""

    psuade_moat: PsuadeMoatConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "PSUADE_MOAT",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
