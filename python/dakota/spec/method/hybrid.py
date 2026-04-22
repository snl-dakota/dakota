"""Generated Pydantic models for method.hybrid"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Union

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodIteratorServerSchedulingMixin,
    MethodThreeOptionalKeywordsMixin,
)


class SequentialMethodNameListConfig(DakotaBaseModel):
    "List of Dakota methods to sequentially or collaboratively run"

    method_name_list: list[str] = DakotaField(
        description="List of Dakota methods to sequentially or collaboratively run",
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
    "Pointers to methods to execute sequantially or collaboratively"

    method_pointer_list: list[str] = DakotaField(
        description="Pointers to methods to execute sequantially or collaboratively",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.method_pointers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
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


class CollaborativeMethodNameListConfig(DakotaBaseModel):
    "List of Dakota methods to sequentially or collaboratively run"

    method_name_list: list[str] = DakotaField(
        description="List of Dakota methods to sequentially or collaboratively run",
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
    "Pointers to methods to execute sequantially or collaboratively"

    method_pointer_list: list[str] = DakotaField(
        description="Pointers to methods to execute sequantially or collaboratively",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.hybrid.method_pointers",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "StringArray",
                }
            ]
        },
    )


class SequentialMethodNameList(DakotaBaseModel):
    "List of Dakota methods to sequentially or collaboratively run"

    method_name_list: SequentialMethodNameListConfig = DakotaField(
        default=...,
        description="List of Dakota methods to sequentially or collaboratively run",
        dakota={"argument": "method_name_list"},
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


class CollaborativeMethodNameList(DakotaBaseModel):
    "List of Dakota methods to sequentially or collaboratively run"

    method_name_list: CollaborativeMethodNameListConfig = DakotaField(
        default=...,
        description="List of Dakota methods to sequentially or collaboratively run",
        dakota={"argument": "method_name_list"},
    )


class SequentialConfig(MethodIteratorServerSchedulingMixin):
    "Methods are run one at a time, in sequence"

    sub_method: Union[SequentialMethodNameList, SequentialMethodPointerList] = (
        DakotaField(
            description="Sub-method Selection",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )


class EmbeddedConfig(MethodIteratorServerSchedulingMixin):
    "A subordinate local method provides periodic refinements to a top-level global method"

    sub_method: Union[GlobalMethodName, GlobalMethodPointer] = DakotaField(
        description="Global Sub-method Selection",
        dakota={"anchor": True, "union_pattern": 4},
    )
    local_sub_method: Union[LocalMethodName, LocalMethodPointer] = DakotaField(
        description="Local Sub-method Selection",
        dakota={"anchor": True, "union_pattern": 4},
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


class CollaborativeConfig(MethodIteratorServerSchedulingMixin):
    "Multiple methods run concurrently and share information"

    sub_method: Union[CollaborativeMethodNameList, CollaborativeMethodPointerList] = (
        DakotaField(
            description="Sub-method Selection",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )


class Sequential(DakotaBaseModel):
    "Methods are run one at a time, in sequence"

    sequential: SequentialConfig = DakotaField(
        description="Methods are run one at a time, in sequence",
        dakota={
            "aliases": ["uncoupled"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_SEQUENTIAL",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Embedded(DakotaBaseModel):
    "A subordinate local method provides periodic refinements to a top-level global method"

    embedded: EmbeddedConfig = DakotaField(
        description="A subordinate local method provides periodic refinements to a top-level global method",
        dakota={
            "aliases": ["coupled"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_EMBEDDED",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Collaborative(DakotaBaseModel):
    "Multiple methods run concurrently and share information"

    collaborative: CollaborativeConfig = DakotaField(
        description="Multiple methods run concurrently and share information",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_COLLABORATIVE",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class HybridConfig(MethodThreeOptionalKeywordsMixin):
    "Strategy in which a set of methods synergistically seek an optimal design"

    strategy: Union[Sequential, Embedded, Collaborative] = DakotaField(
        description="Hybrid Method Type", dakota={"anchor": True, "union_pattern": 4}
    )


class HybridSelection(MethodSelection):
    "Generated model for HybridSelection"

    hybrid: HybridConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "HYBRID",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
