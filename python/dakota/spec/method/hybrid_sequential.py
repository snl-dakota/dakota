"""Pydantic models for the hybrid_sequential method."""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Union

from dakota.spec.shared.misc import (
    MethodIteratorServerSchedulingMixin,
    MethodThreeOptionalKeywordsMixin,
)


class SequentialMethodNameListConfig(DakotaBaseModel):
    "List of Dakota methods to sequentially run"

    method_name_list: list[str] = DakotaField(
        description="List of Dakota methods to sequentially run",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.method_names",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )
    model_pointer_list: list[str] | None = DakotaField(
        default=None,
        description="Associate models with method names",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.model_pointers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )


class SequentialMethodPointerList(DakotaBaseModel):
    "Pointers to methods to execute sequentially"

    method_pointer_list: list[str] = DakotaField(
        description="Pointers to methods to execute sequentially",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "method.hybrid.method_pointers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ],
        },
    )


class SequentialMethodNameList(DakotaBaseModel):
    "List of Dakota methods to sequentially run"

    method_name_list: SequentialMethodNameListConfig = DakotaField(
        default=...,
        description="List of Dakota methods to sequentially run",
        dakota={"argument": "method_name_list"},
    )


class SequentialConfig(MethodIteratorServerSchedulingMixin):
    "Methods are run one at a time, in sequence"

    sub_method: Union[SequentialMethodNameList, SequentialMethodPointerList] = (
        DakotaField(
            description="Sub-method Selection",
            dakota={"anchor": True, "union_pattern": 4, "pointer_union": True},
        )
    )


class HybridSequentialConfig(MethodThreeOptionalKeywordsMixin, SequentialConfig):
    "Methods are run one at a time, in sequence"


class HybridSequentialSelection(MethodSelection):
    "Methods are run one at a time, in sequence"

    hybrid_sequential: HybridSequentialConfig = DakotaField(
        description="Methods are run one at a time, in sequence",
        dakota={
            "aliases": ["hybrid_uncoupled"],
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HYBRID",
                    "ir_value_type": "unsigned short",
                },
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_SEQUENTIAL",
                    "ir_value_type": "unsigned short",
                },
            ],
        },
    )
