"""Generated Pydantic models for method.pareto_set"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodIteratorServerSchedulingMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ParetoSetMethodNameConfig(DakotaBaseModel):
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
            "aliases": ["opt_model_pointer"],
            "materialization": [
                {
                    "ir_key": "method.sub_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class ParetoSetMethodPointer(DakotaBaseModel):
    "Pointer to optimization or least-squares sub-method"

    method_pointer: str = DakotaField(
        description="Pointer to optimization or least-squares sub-method",
        dakota={
            "block_pointer": "method",
            "aliases": ["opt_method_pointer"],
            "materialization": [
                {
                    "ir_key": "method.sub_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class RandomWeightSets(DakotaBaseModel):
    "Number of random weighting sets"

    count: int = DakotaField(
        default=0,
        description="Number of random weighting sets",
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


class ParetoSetMethodName(DakotaBaseModel):
    "Specify sub-method by name"

    method_name: ParetoSetMethodNameConfig = DakotaField(
        default=...,
        description="Specify sub-method by name",
        dakota={"argument": "method_name", "aliases": ["opt_method_name"]},
    )


class ParetoSetConfig(
    MethodThreeOptionalKeywordsMixin, MethodIteratorServerSchedulingMixin
):
    "Pareto set optimization"

    sub_method: Union[ParetoSetMethodName, ParetoSetMethodPointer] = DakotaField(
        description="Sub-method Selection", dakota={"anchor": True, "union_pattern": 4}
    )
    random_weight_sets: RandomWeightSets | None = DakotaField(
        default=None,
        description="Number of random weighting sets",
        dakota={"argument": "count"},
    )
    weight_sets: list[DakotaFloat] | None = DakotaField(
        default=None,
        description="List of user-specified weighting sets",
        dakota={
            "aliases": ["multi_objective_weight_sets"],
            "materialization": [
                {
                    "ir_key": "method.concurrent.parameter_sets",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "RealVector",
                }
            ],
        },
    )


class ParetoSetSelection(MethodSelection):
    "Generated model for ParetoSetSelection"

    pareto_set: ParetoSetConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "PARETO_SET",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
