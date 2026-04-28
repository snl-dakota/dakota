"""Generated Pydantic models for method.multi_start"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodIteratorServerSchedulingMixin,
    MethodThreeOptionalKeywordsMixin,
)


class MultiStartMethodNameConfig(DakotaBaseModel):
    "Specify sub-method by name"

    method_name: str = DakotaField(
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
    model_pointer: str | None = DakotaField(
        default=None,
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.sub_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class MultiStartMethodPointer(DakotaBaseModel):
    "Pointer to sub-method to run from each starting point"

    method_pointer: str = DakotaField(
        description="Pointer to sub-method to run from each starting point",
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


class RandomStarts(DakotaBaseModel):
    "Number of random starting points"

    count: int = DakotaField(
        default=0,
        description="Number of random starting points",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.concurrent.random_jobs",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "int",
                }
            ]
        },
    )
    seed: int | None = DakotaField(
        default=None,
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


class MultiStartMethodName(DakotaBaseModel):
    "Specify sub-method by name"

    method_name: MultiStartMethodNameConfig = DakotaField(
        default=...,
        description="Specify sub-method by name",
        dakota={"argument": "method_name"},
    )


class MultiStartConfig(
    MethodThreeOptionalKeywordsMixin, MethodIteratorServerSchedulingMixin
):
    "Multi-Start Optimization Method"

    sub_method: Union[MultiStartMethodName, MultiStartMethodPointer] = DakotaField(
        description="Sub-method Selection", dakota={"anchor": True, "union_pattern": 4}
    )
    random_starts: RandomStarts | None = DakotaField(
        default=None,
        description="Number of random starting points",
        dakota={"argument": "count"},
    )
    starting_points: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="List of user-specified starting points",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.concurrent.parameter_sets",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ]
        },
    )


class MultiStartSelection(MethodSelection):
    "Generated model for MultiStartSelection"

    multi_start: MultiStartConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "MULTI_START",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
