"""Generated Pydantic models for method.multidim_parameter_study"""

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


class MultidimParameterStudyConfig(
    MethodThreeOptionalKeywordsMixin, MethodOptionalModelPointerMixin
):
    "Samples variables on full factorial grid of study points"

    _VALIDATION_RULES: ClassVar[List[ValidationRule]] = [
        CheckNonnegativeList(
            context="multidimparameterstudyconfig", list_field="partitions"
        ),
    ]

    partitions: list[int] = DakotaField(
        description="Samples variables on full factorial grid of study points",
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


class MultidimParameterStudySelection(MethodSelection):
    "Generated model for MultidimParameterStudySelection"

    multidim_parameter_study: MultidimParameterStudyConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTIDIM_PARAMETER_STUDY",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
