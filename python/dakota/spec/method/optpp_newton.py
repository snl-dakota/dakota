"""Generated Pydantic models for method.optpp_newton"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptppGradBasedOptsMixin,
    MethodOptppNewtonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class OptppNewtonConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodOptppNewtonOptsMixin,
    MethodOptppGradBasedOptsMixin,
):
    "Newton method based optimization"

    pass


class OptppNewtonSelection(MethodSelection):
    "Generated model for OptppNewtonSelection"

    optpp_newton: OptppNewtonConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OPTPP_NEWTON",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
