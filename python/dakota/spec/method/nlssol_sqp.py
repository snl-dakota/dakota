"""Generated Pydantic models for method.nlssol_sqp"""

from __future__ import annotations

from ..base import DakotaField
from .base import MethodSelection

# Cross-module model imports
from dakota.spec.shared.misc import (
    MethodNpsolCommonOptsMixin,
    MethodThreeOptionalKeywordsMixin,
)


class NlssolSqpConfig(MethodThreeOptionalKeywordsMixin, MethodNpsolCommonOptsMixin):
    """Generated model for NlssolSqpConfig"""

    pass


class NlssolSqpSelection(MethodSelection):
    """Generated model for NlssolSqpSelection"""

    nlssol_sqp: NlssolSqpConfig = DakotaField(
        dakota={
            "materialization": [
                {
                    "ir_key": "method.algorithm",
                    "stored_value": "NLSSOL_SQP",
                    "storage_type": "PRESENCE_ENUM",
                    "ir_value_type": "unsigned short",
                }
            ]
        }
    )
