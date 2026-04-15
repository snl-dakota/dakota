"""Generated Pydantic models for method.dot_sqp"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodDotCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class DotSqpConfig(MethodThreeOptionalKeywordsMixin, MethodDotCommonOptsMixin):
    """Generated model for DotSqpConfig"""

    pass


class DotSqpSelection(MethodSelection):
    """Generated model for DotSqpSelection"""

    dot_sqp: DotSqpConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DOT_SQP",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
