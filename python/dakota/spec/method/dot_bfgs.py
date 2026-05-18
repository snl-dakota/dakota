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
    "DOT BFGS optimization method"

    pass


class DotBfgsSelection(MethodSelection):
    "Generated model for DotBfgsSelection"

    dot_bfgs: DotBfgsConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "storage_type": "PRESENCE_ENUM",
                    "stored_value": "DOT_BFGS",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
