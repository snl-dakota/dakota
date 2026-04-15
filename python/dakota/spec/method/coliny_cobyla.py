"""Generated Pydantic models for method.coliny_cobyla"""

from __future__ import annotations

from ..base import DakotaField, DakotaFloat
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
    MethodThreeOptionalKeywordsMixin,
)


class ColinyCobylaConfig(
    MethodThreeOptionalKeywordsMixin,
    ColinyCommonOptsMixin,
    MethodOptionalModelPointerMixin,
):
    """Generated model for ColinyCobylaConfig"""

    initial_delta: DakotaFloat = DakotaField(
        default=-1.0,
        description="Reasonable initial changes to optimization variables",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.initial_delta",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )
    variable_tolerance: DakotaFloat = DakotaField(
        default=-1.0,
        description="Required or expected accuracy in optimization variables.",
        dakota={
            "materialization": [
                {
                    "ir_key": "method.coliny.variable_tolerance",
                    "storage_type": "DIRECT_VALUE",
                    "ir_value_type": "Real",
                }
            ]
        },
    )


class ColinyCobylaSelection(MethodSelection):
    """Generated model for ColinyCobylaSelection"""

    coliny_cobyla: ColinyCobylaConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "COLINY_COBYLA",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
