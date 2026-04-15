"""Generated Pydantic models for method.dot_mmfd"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodDotCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class DotMmfdConfig(MethodThreeOptionalKeywordsMixin, MethodDotCommonOptsMixin):
    """Generated model for DotMmfdConfig"""

    pass


class DotMmfdSelection(MethodSelection):
    """Generated model for DotMmfdSelection"""

    dot_mmfd: DotMmfdConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DOT_MMFD",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
