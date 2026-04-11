"""Generated Pydantic models for method.dot_slp"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodDotCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class DotSlpConfig(MethodThreeOptionalKeywordsMixin, MethodDotCommonOptsMixin):
    """Generated model for DotSlpConfig"""

    pass


class DotSlpSelection(MethodSelection):
    """Generated model for DotSlpSelection"""

    dot_slp: DotSlpConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "DOT_SLP",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
