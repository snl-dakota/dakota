"""Generated Pydantic models for method.vector_parameter_study"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class FinalPoint(DakotaBaseModel):
    """Generated model for FinalPoint"""

    final_point: list[DakotaFloat] = DakotaField(
        description="Final variable values defining vector in vector parameter study",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.parameter_study.final_point",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class StepVector(DakotaBaseModel):
    """Generated model for StepVector"""

    step_vector: list[DakotaFloat] = DakotaField(
        description="Size of step for each variable",
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


class VectorParameterStudyConfig(
    MethodThreeOptionalKeywordsMixin, MethodOptionalModelPointerMixin
):
    """Generated model for VectorParameterStudyConfig"""

    step_control: Union[FinalPoint, StepVector] = DakotaField(
        description="Step Control", dakota={"anchor": True, "union_pattern": 4}
    )
    num_steps: int = DakotaField(
        description="Number of sampling steps along the vector in a vector parameter study",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.parameter_study.num_steps",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )


class VectorParameterStudySelection(MethodSelection):
    """Generated model for VectorParameterStudySelection"""

    vector_parameter_study: VectorParameterStudyConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "VECTOR_PARAMETER_STUDY",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
