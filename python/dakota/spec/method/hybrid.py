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
    """Generated model for SequentialMethodNameListConfig"""

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
    """Generated model for SequentialMethodPointerList"""

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
    """Generated model for GlobalMethodNameConfig"""

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
    """Generated model for GlobalMethodPointer"""

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
    """Generated model for LocalMethodNameConfig"""

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
    """Generated model for LocalMethodPointer"""

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
    """Generated model for CollaborativeMethodNameListConfig"""

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
    """Generated model for CollaborativeMethodPointerList"""

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
    """Generated model for SequentialMethodNameList"""

    method_name_list: SequentialMethodNameListConfig = DakotaField(
        default=...,
        description="List of Dakota methods to sequentially or collaboratively run",
        dakota={"argument": "method_name_list"},
    )


class GlobalMethodName(DakotaBaseModel):
    """Generated model for GlobalMethodName"""

    global_method_name: GlobalMethodNameConfig = DakotaField(
        default=...,
        description="Specify the global method by Dakota name",
        dakota={"argument": "name"},
    )


class LocalMethodName(DakotaBaseModel):
    """Generated model for LocalMethodName"""

    local_method_name: LocalMethodNameConfig = DakotaField(
        default=...,
        description="Specify the local method by Dakota name",
        dakota={"argument": "name"},
    )


class CollaborativeMethodNameList(DakotaBaseModel):
    """Generated model for CollaborativeMethodNameList"""

    method_name_list: CollaborativeMethodNameListConfig = DakotaField(
        default=...,
        description="List of Dakota methods to sequentially or collaboratively run",
        dakota={"argument": "method_name_list"},
    )


class SequentialConfig(MethodIteratorServerSchedulingMixin):
    """Generated model for SequentialConfig"""

    sub_method: Union[SequentialMethodNameList, SequentialMethodPointerList] = (
        DakotaField(
            description="Sub-method Selection",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )


class EmbeddedConfig(MethodIteratorServerSchedulingMixin):
    """Generated model for EmbeddedConfig"""

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
    """Generated model for CollaborativeConfig"""

    sub_method: Union[CollaborativeMethodNameList, CollaborativeMethodPointerList] = (
        DakotaField(
            description="Sub-method Selection",
            dakota={"anchor": True, "union_pattern": 4},
        )
    )


class Sequential(DakotaBaseModel):
    """Generated model for Sequential"""

    sequential: SequentialConfig = DakotaField(
        description="Methods are run one at a time, in sequence",
        dakota={
            "aliases": ["uncoupled"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_SEQUENTIAL",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Embedded(DakotaBaseModel):
    """Generated model for Embedded"""

    embedded: EmbeddedConfig = DakotaField(
        description="A subordinate local method provides periodic refinements to a top-level global method",
        dakota={
            "aliases": ["coupled"],
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_EMBEDDED",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ],
        },
    )


class Collaborative(DakotaBaseModel):
    """Generated model for Collaborative"""

    collaborative: CollaborativeConfig = DakotaField(
        description="Multiple methods run concurrently and share information",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "stored_value": "SUBMETHOD_COLLABORATIVE",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class HybridConfig(MethodThreeOptionalKeywordsMixin):
    """Generated model for HybridConfig"""

    strategy: Union[Sequential, Embedded, Collaborative] = DakotaField(
        description="Hybrid Method Type", dakota={"anchor": True, "union_pattern": 4}
    )


class HybridSelection(MethodSelection):
    """Generated model for HybridSelection"""

    hybrid: HybridConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "HYBRID",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
