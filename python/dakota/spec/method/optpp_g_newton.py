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
    """Generated model for OptppGNewtonConfig"""

    pass


class OptppGNewtonSelection(MethodSelection):
    """Generated model for OptppGNewtonSelection"""

    optpp_g_newton: OptppGNewtonConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "OPTPP_G_NEWTON",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
