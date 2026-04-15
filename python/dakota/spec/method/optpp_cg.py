"""Generated Pydantic models for method.optpp_cg"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodOptppGradBasedOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class OptppCgConfig(MethodThreeOptionalKeywordsMixin, MethodOptppGradBasedOptsMixin):
    """Generated model for OptppCgConfig"""

    pass


class OptppCgSelection(MethodSelection):
    """Generated model for OptppCgSelection"""

    optpp_cg: OptppCgConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "OPTPP_CG",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
