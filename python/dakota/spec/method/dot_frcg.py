"""Generated Pydantic models for method.dot_frcg"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodDotCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class DotFrcgConfig(MethodThreeOptionalKeywordsMixin, MethodDotCommonOptsMixin):
    """Generated model for DotFrcgConfig"""

    pass


class DotFrcgSelection(MethodSelection):
    """Generated model for DotFrcgSelection"""

    dot_frcg: DotFrcgConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "DOT_FRCG",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
