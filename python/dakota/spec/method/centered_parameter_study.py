"""Generated Pydantic models for method.centered_parameter_study"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class CenteredParameterStudyConfig(
    MethodThreeOptionalKeywordsMixin, MethodOptionalModelPointerMixin
):
    """Generated model for CenteredParameterStudyConfig"""

    step_vector: list[DakotaFloat] = DakotaField(
        description="Size of steps to be taken in each dimension of a centered parameter study",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.parameter_study.step_vector",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )
    steps_per_variable: list[int] = DakotaField(
        description="Number of steps to take in each dimension of a centered parameter study",
        dakota={
            "aliases": ["deltas_per_variable"],
            "materialization": [
                {
                    "ir_key": "method.parameter_study.steps_per_variable",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "IntVector",
                }
            ],
        },
    )


class CenteredParameterStudySelection(MethodSelection):
    """Generated model for CenteredParameterStudySelection"""

    centered_parameter_study: CenteredParameterStudyConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "CENTERED_PARAMETER_STUDY",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
