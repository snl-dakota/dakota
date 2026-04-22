"""Generated Pydantic models for method.optpp_q_newton"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptppGradBasedOptsMixin,
    MethodOptppNewtonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class OptppQNewtonConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodOptppNewtonOptsMixin,
    MethodOptppGradBasedOptsMixin,
):
    "Quasi-Newton optimization method"

    pass


class OptppQNewtonSelection(MethodSelection):
    "Generated model for OptppQNewtonSelection"

    optpp_q_newton: OptppQNewtonConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OPTPP_Q_NEWTON",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
