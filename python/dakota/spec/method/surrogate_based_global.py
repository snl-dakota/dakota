"""Generated Pydantic models for method.surrogate_based_global"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodMaxIterationsContext1Mixin,
    MethodThreeOptionalKeywordsMixin,
)


class SurrogateBasedGlobalMethodPointer(DakotaBaseModel):
    "Pointer to sub-method to apply to a surrogate or branch-and-bound sub-problem"

    method_pointer: str = DakotaField(
        description="Pointer to sub-method to apply to a surrogate or branch-and-bound sub-problem",
        dakota={
            "block_pointer": "method",
            "aliases": ["approx_method_pointer"],
            "materialization": [
                {
                    "ir_key": "method.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class SurrogateBasedGlobalMethodName(DakotaBaseModel):
    "Specify sub-method by name"

    method_name: str = DakotaField(
        description="Specify sub-method by name",
        dakota={
            "aliases": ["approx_method_name"],
            "materialization": [
                {
                    "ir_key": "method.sub_method_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class SurrogateBasedGlobalConfig(
    MethodThreeOptionalKeywordsMixin, MethodMaxIterationsContext1Mixin
):
    "Adaptive Global Surrogate-Based Optimization"

    sub_method: Union[
        SurrogateBasedGlobalMethodPointer, SurrogateBasedGlobalMethodName
    ] = DakotaField(
        description="Sub-method Selection", dakota={"anchor": True, "union_pattern": 4}
    )
    model_pointer: str = DakotaField(
        description="Identifier for model block to be used by a method",
        dakota={
            "block_pointer": "model",
            "aliases": ["approx_model_pointer"],
            "materialization": [
                {
                    "ir_key": "method.model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )
    replace_points: Literal[True] | None = DakotaField(
        default=None,
        description="(Recommended) Replace points in the surrogate training set, instead of appending",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sbg.replace_points",
                    "storage_type": "PRESENCE_TRUE",
                    "ir_value_type": "bool",
                }
            ]
        },
    )


class SurrogateBasedGlobalSelection(MethodSelection):
    "Generated model for SurrogateBasedGlobalSelection"

    surrogate_based_global: SurrogateBasedGlobalConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SURROGATE_BASED_GLOBAL",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
