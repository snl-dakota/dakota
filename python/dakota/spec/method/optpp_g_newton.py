"""Generated Pydantic models for method.optpp_g_newton"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptppGradBasedOptsMixin,
    MethodOptppNewtonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class OptppGNewtonConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodOptppNewtonOptsMixin,
    MethodOptppGradBasedOptsMixin,
):
    "Newton method based least-squares calbration"

    pass


class OptppGNewtonSelection(MethodSelection):
    "Generated model for OptppGNewtonSelection"

    optpp_g_newton: OptppGNewtonConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OPTPP_G_NEWTON",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
