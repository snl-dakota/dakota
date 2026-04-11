"""Generated Pydantic models for method.dot_bfgs"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodDotCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class DotBfgsConfig(MethodThreeOptionalKeywordsMixin, MethodDotCommonOptsMixin):
    """Generated model for DotBfgsConfig"""

    pass


class DotBfgsSelection(MethodSelection):
    """Generated model for DotBfgsSelection"""

    dot_bfgs: DotBfgsConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "DOT_BFGS",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
