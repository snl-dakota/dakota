"""Pydantic models for the hybrid_embedded method."""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Union

from dakota.spec.shared.misc import (
    MethodIteratorServerSchedulingMixin,
    MethodThreeOptionalKeywordsMixin,
)


class GlobalMethodNameConfig(DakotaBaseModel):
    "Specify the global method by Dakota name"

    name: str = DakotaField(
        description="Specify the global method by Dakota name",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.global_method_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    global_model_pointer: str | None = DakotaField(
        default=None,
        description="Pointer to model used by global method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.hybrid.global_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class GlobalMethodPointer(DakotaBaseModel):
    "Pointer to global method"

    global_method_pointer: str = DakotaField(
        description="Pointer to global method",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "method.hybrid.global_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class LocalMethodNameConfig(DakotaBaseModel):
    "Specify the local method by Dakota name"

    name: str = DakotaField(
        description="Specify the local method by Dakota name",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.local_method_name",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ]
        },
    )
    local_model_pointer: str | None = DakotaField(
        default=None,
        description="Pointer to model used by local method",
        dakota={
            "block_pointer": "model",
            "materialization": [
                {
                    "ir_key": "method.hybrid.local_model_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class LocalMethodPointer(DakotaBaseModel):
    "Pointer to local method"

    local_method_pointer: str = DakotaField(
        description="Pointer to local method",
        dakota={
            "block_pointer": "method",
            "materialization": [
                {
                    "ir_key": "method.hybrid.local_method_pointer",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "String",
                }
            ],
        },
    )


class GlobalMethodName(DakotaBaseModel):
    "Specify the global method by Dakota name"

    global_method_name: GlobalMethodNameConfig = DakotaField(
        default=...,
        description="Specify the global method by Dakota name",
        dakota={"argument": "name"},
    )


class LocalMethodName(DakotaBaseModel):
    "Specify the local method by Dakota name"

    local_method_name: LocalMethodNameConfig = DakotaField(
        default=...,
        description="Specify the local method by Dakota name",
        dakota={"argument": "name"},
    )


class EmbeddedConfig(MethodIteratorServerSchedulingMixin):
    "A subordinate local method provides periodic refinements to a top-level global method"

    sub_method: Union[GlobalMethodName, GlobalMethodPointer] = DakotaField(
        description="Global Sub-method Selection",
        dakota={"anchor": True, "union_pattern": 4, "pointer_union": True},
    )
    local_sub_method: Union[LocalMethodName, LocalMethodPointer] = DakotaField(
        description="Local Sub-method Selection",
        dakota={"anchor": True, "union_pattern": 4, "pointer_union": True},
    )
    local_search_probability: DakotaFloat = DakotaField(
        default=0.1,
        description="Probability of executing local searches",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.local_search_probability",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class HybridEmbeddedConfig(MethodThreeOptionalKeywordsMixin, EmbeddedConfig):
    "A subordinate local method provides periodic refinements to a top-level global method"


class HybridEmbeddedSelection(MethodSelection):
    "A subordinate local method provides periodic refinements to a top-level global method"

    hybrid_embedded: HybridEmbeddedConfig = DakotaField(
        description="A subordinate local method provides periodic refinements to a top-level global method",
        dakota={
            "aliases": ["hybrid_coupled"],
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
                    "stored_value": "SUBMETHOD_EMBEDDED",
                    "ir_value_type": "unsigned short",
                },
            ],
        },
    )
