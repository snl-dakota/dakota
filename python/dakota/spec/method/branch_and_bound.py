"""Generated Pydantic models for method.branch_and_bound"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    DefaultScalingMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class BranchAndBoundMethodPointer(DakotaBaseModel):
    """Generated model for BranchAndBoundMethodPointer"""

    method_pointer: str = DakotaField(
        description="Pointer to sub-method to apply to a surrogate or branch-and-bound sub-problem",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "method.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class BranchAndBoundMethodNameConfig(MethodOptionalModelPointerMixin):
    """Generated model for BranchAndBoundMethodNameConfig"""

    name: str = DakotaField(
        description="Specify sub-method by name",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )


class BranchAndBoundMethodName(DakotaBaseModel):
    """Generated model for BranchAndBoundMethodName"""

    method_name: BranchAndBoundMethodNameConfig = DakotaField(
        default=...,
        description="Specify sub-method by name",
        dakota={"argument": "name"},
    )


class BranchAndBoundConfig(MethodThreeOptionalKeywordsMixin, DefaultScalingMixin):
    """Generated model for BranchAndBoundConfig"""

    sub_method: Union[BranchAndBoundMethodPointer, BranchAndBoundMethodName] = (
        DakotaField(
            description="Local Optimizer Selection",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )


class BranchAndBoundSelection(MethodSelection):
    """Generated model for BranchAndBoundSelection"""

    branch_and_bound: BranchAndBoundConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "BRANCH_AND_BOUND",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
