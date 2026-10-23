"""Pydantic models for the hybrid_collaborative method."""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField
from .base import MethodSelection
from typing import Union

from dakota.spec.shared.misc import (
    MethodIteratorServerSchedulingMixin,
    MethodThreeOptionalKeywordsMixin,
)


class CollaborativeMethodNameListConfig(DakotaBaseModel):
    "List of Dakota methods to collaboratively run"

    method_name_list: list[str] = DakotaField(
        description="List of Dakota methods to collaboratively run",
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


class CollaborativeMethodPointerList(DakotaBaseModel):
    "Pointers to methods to execute collaboratively"

    method_pointer_list: list[str] = DakotaField(
        description="Pointers to methods to execute collaboratively",
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


class CollaborativeMethodNameList(DakotaBaseModel):
    "List of Dakota methods to collaboratively run"

    method_name_list: CollaborativeMethodNameListConfig = DakotaField(
        default=...,
        description="List of Dakota methods to collaboratively run",
        dakota={"argument": "method_name_list"},
    )


class CollaborativeConfig(MethodIteratorServerSchedulingMixin):
    "Multiple methods run concurrently and share information"

    sub_method: Union[CollaborativeMethodNameList, CollaborativeMethodPointerList] = (
        DakotaField(
            description="Sub-method Selection",
            dakota={"anchor": True, "union_pattern": 4, "pointer_union": True},
        )
    )


class HybridCollaborativeConfig(MethodThreeOptionalKeywordsMixin, CollaborativeConfig):
    "Multiple methods run concurrently and share information"


class HybridCollaborativeSelection(MethodSelection):
    "Multiple methods run concurrently and share information"

    hybrid_collaborative: HybridCollaborativeConfig = DakotaField(
        description="Multiple methods run concurrently and share information",
        dakota={
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
                    "stored_value": "SUBMETHOD_COLLABORATIVE",
                    "ir_value_type": "unsigned short",
                },
            ],
        },
    )
