"""Generated Pydantic models for method.optpp_fd_newton"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptppGradBasedOptsMixin,
    MethodOptppNewtonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class OptppFdNewtonConfig(
    MethodThreeOptionalKeywordsMixin,
    MethodOptppNewtonOptsMixin,
    MethodOptppGradBasedOptsMixin,
):
    "Finite Difference Newton optimization method"

    pass


class OptppFdNewtonSelection(MethodSelection):
    "Generated model for OptppFdNewtonSelection"

    optpp_fd_newton: OptppFdNewtonConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OPTPP_FD_NEWTON",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
