"""Generated Pydantic models for method.richardson_extrap"""

from __future__ import annotations

from ..base import DakotaBaseModel, DakotaField, DakotaFloat
from .base import MethodSelection
from typing import Literal, Union

# Cross-module model imports
from dakota.spec.shared.core import MethodConvergenceTolMixin
from dakota.spec.shared.misc import (
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class EstimateOrder(DakotaBaseModel):
    """Generated model for EstimateOrder"""

    estimate_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_ESTIMATE_ORDER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ConvergeOrder(DakotaBaseModel):
    """Generated model for ConvergeOrder"""

    converge_order: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_CONVERGE_ORDER",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class ConvergeQoi(DakotaBaseModel):
    """Generated model for ConvergeQoi"""

    converge_qoi: Literal[True] = DakotaField(
        default=True,
        dakota={
            "materialization": [
                {
                    "ir_key": "method.sub_method",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "SUBMETHOD_CONVERGE_QOI",
                    "ir_value_type": "unsigned short",
                }
            ]
        },
    )


class RichardsonExtrapConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodConvergenceTolMixin,
    MethodMaxIterationsContext1Mixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for RichardsonExtrapConfig"""

    mode: Union[EstimateOrder, ConvergeOrder, ConvergeQoi] = DakotaField(
        description="Verification Mode", dakota={"anchor": True, "union_pattern": 4}
    )
    refinement_rate: DakotaFloat = DakotaField(
        default=2.0,
        description="Rate at which the state variables are refined",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.verification.refinement_rate",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class RichardsonExtrapSelection(MethodSelection):
    """Generated model for RichardsonExtrapSelection"""

    richardson_extrap: RichardsonExtrapConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "RICHARDSON_EXTRAP",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
